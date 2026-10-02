//! Process-lifetime channel worker pool.
//!
//! Agent inference workers spawned by `kylinbot-channels`'
//! [`run_message_dispatch_loop`] are detached into this pool. The pool is
//! constructed once by `daemon::run` and reused across every supervisor
//! restart. Cancelling the channels supervisor (e.g. during a config reload)
//! does not abort the workers — they keep running with the resources they
//! captured at spawn time, finish the in-flight inference, and deliver the
//! reply through the channel `Arc` they hold.
//!
//! The shared `Semaphore` enforces a global concurrency cap that survives
//! reloads: if a reload happens while N workers are in flight, the new
//! dispatch loop sees only `max_in_flight - N` free permits until those
//! workers drain.
//!
//! [`run_message_dispatch_loop`]: kylinbot-channels::orchestrator::run_message_dispatch_loop

use std::sync::Arc;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::time::{Duration, Instant};

/// Daemon-level worker pool for channel message dispatch.
///
/// The pool itself is cheap (two `Arc`s); `clone()` is the intended way to
/// share it across subsystems. Workers spawned via [`Self::spawn_worker`] run
/// on the global tokio runtime as detached tasks.
#[derive(Clone)]
pub struct ChannelWorkerPool {
    semaphore: Arc<tokio::sync::Semaphore>,
    in_flight: Arc<AtomicUsize>,
}

impl ChannelWorkerPool {
    /// Create a pool with `max_in_flight` concurrent workers (minimum 1).
    pub fn new(max_in_flight: usize) -> Self {
        Self {
            semaphore: Arc::new(tokio::sync::Semaphore::new(max_in_flight.max(1))),
            in_flight: Arc::new(AtomicUsize::new(0)),
        }
    }

    /// Acquire a pool slot and spawn `fut` to run on the global tokio runtime,
    /// holding the slot until the future completes.
    ///
    /// **Awaits a permit** — back-pressure flows up to the caller (typically
    /// the channel message dispatch loop). The spawned future itself runs
    /// detached: no `JoinHandle` is retained, so cancelling the caller (or
    /// a supervisor that owns the caller's future) does not abort the worker.
    /// This is what lets in-flight agent inference survive a channels reload.
    pub async fn spawn_worker<F>(&self, fut: F)
    where
        F: std::future::Future<Output = ()> + Send + 'static,
    {
        let permit = match self.semaphore.clone().acquire_owned().await {
            Ok(p) => p,
            Err(_) => {
                tracing::warn!("ChannelWorkerPool: semaphore closed; dropping worker");
                return;
            }
        };
        self.in_flight.fetch_add(1, Ordering::Relaxed);
        let counter = self.in_flight.clone();
        tokio::spawn(async move {
            let _permit = permit;
            fut.await;
            counter.fetch_sub(1, Ordering::Relaxed);
        });
    }

    /// Number of workers currently executing.
    pub fn active(&self) -> usize {
        self.in_flight.load(Ordering::Relaxed)
    }

    /// Poll-wait until all workers complete, bounded by `timeout`.
    ///
    /// Returns `true` if the pool fully drained within the deadline,
    /// `false` if it timed out (in which case some workers are still
    /// running and will continue on the tokio runtime).
    pub async fn drain_with_timeout(&self, timeout: Duration) -> bool {
        let deadline = Instant::now() + timeout;
        while self.active() > 0 && Instant::now() < deadline {
            tokio::time::sleep(Duration::from_millis(100)).await;
        }
        self.active() == 0
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::atomic::AtomicBool;

    #[tokio::test]
    async fn spawn_worker_runs_to_completion() {
        let pool = ChannelWorkerPool::new(4);
        let done = Arc::new(AtomicBool::new(false));
        let done_c = done.clone();
        pool.spawn_worker(async move {
            tokio::time::sleep(Duration::from_millis(50)).await;
            done_c.store(true, Ordering::SeqCst);
        })
        .await;
        // Active is incremented synchronously at spawn time, so drain blocks
        // until the worker truly finishes.
        assert!(pool.drain_with_timeout(Duration::from_secs(2)).await);
        assert!(done.load(Ordering::SeqCst));
    }

    #[tokio::test]
    async fn active_count_tracks_in_flight() {
        let pool = ChannelWorkerPool::new(8);
        // Each spawn synchronously bumps the counter; using a Semaphore as
        // a multi-waiter release gate (Notify::notify_one stores only one
        // outstanding permit and would deadlock here).
        let release = Arc::new(tokio::sync::Semaphore::new(0));
        for _ in 0..3 {
            let release = release.clone();
            pool.spawn_worker(async move {
                let _ = release.acquire_owned().await;
            })
            .await;
        }
        assert_eq!(pool.active(), 3);
        release.add_permits(3);
        assert!(pool.drain_with_timeout(Duration::from_secs(2)).await);
        assert_eq!(pool.active(), 0);
    }

    #[tokio::test]
    async fn drain_returns_false_on_timeout() {
        let pool = ChannelWorkerPool::new(2);
        let release = Arc::new(tokio::sync::Semaphore::new(0));
        let release_c = release.clone();
        pool.spawn_worker(async move {
            let _ = release_c.acquire_owned().await;
        })
        .await;
        // Worker is blocked; drain must time out without completing.
        assert!(!pool.drain_with_timeout(Duration::from_millis(150)).await);
        // Cleanup so the test process exits cleanly.
        release.add_permits(1);
        assert!(pool.drain_with_timeout(Duration::from_secs(1)).await);
    }

    #[tokio::test]
    async fn spawn_worker_blocks_when_pool_is_full() {
        let pool = ChannelWorkerPool::new(2);
        let release = Arc::new(tokio::sync::Semaphore::new(0));
        // Fill the pool with 2 blocking workers.
        for _ in 0..2 {
            let release = release.clone();
            pool.spawn_worker(async move {
                let _ = release.acquire_owned().await;
            })
            .await;
        }
        assert_eq!(pool.active(), 2);

        // Spawning a 3rd worker must block; verify via timeout.
        let pool_c = pool.clone();
        let release_c = release.clone();
        let spawn_task = tokio::spawn(async move {
            pool_c
                .spawn_worker(async move {
                    let _ = release_c.acquire_owned().await;
                })
                .await;
        });
        // The 3rd spawn_worker call should NOT have resolved within this window.
        let waited = tokio::time::timeout(Duration::from_millis(100), spawn_task).await;
        assert!(waited.is_err(), "spawn_worker should block when pool is full");

        // Release one slot; the queued spawn should now proceed.
        release.add_permits(3);
        assert!(pool.drain_with_timeout(Duration::from_secs(2)).await);
    }
}
