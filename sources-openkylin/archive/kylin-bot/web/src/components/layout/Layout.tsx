import { useState } from 'react';
import { useEffect } from 'react';
import { Outlet, useLocation } from 'react-router-dom';
import Sidebar from '@/components/layout/Sidebar';
import Header from '@/components/layout/Header';
import LeftRail from '@/components/layout/LeftRail';
import { ErrorBoundary } from '@/App';
import { useTheme } from '@/hooks/useTheme';
import { resolveNavMode } from '@/components/layout/navModes';
import { AgentNewSessionProvider, useAgentNewSession } from '@/contexts/AgentNewSessionContext';
import { t } from '@/lib/i18n';
import { assetBasePath } from '@/lib/basePath';

export interface AgentPageOutletContext {
  setAgentGenerating: (value: boolean) => void;
}

function LayoutShell() {
  const { pathname } = useLocation();
  const { requestNewSession } = useAgentNewSession();
  const [sidebarOpen, setSidebarOpen] = useState(false);
  const [isAgentGenerating, setIsAgentGenerating] = useState(false);
  const [tipVisible, setTipVisible] = useState(false);
  const [tipText, setTipText] = useState('');
  const { resolvedTheme } = useTheme();
  const isLight = resolvedTheme === 'light';
  const currentMode = resolveNavMode(pathname);
  const desktopOffsetClass = currentMode.key === 'agent' ? 'md:ml-14' : 'md:ml-[18.5rem]';

  const showGeneratingTip = () => {
    setTipText(t('agent.generating_block_tip'));
    setTipVisible(true);
  };

  const handleBlockedOrRun = (action: () => void) => {
    if (isAgentGenerating) {
      showGeneratingTip();
      return;
    }
    action();
  };

  // Close sidebar on route change (mobile navigation)
  useEffect(() => {
    setSidebarOpen(false);
  }, [pathname]);

  useEffect(() => {
    if (!tipVisible) return;
    const timer = window.setTimeout(() => {
      setTipVisible(false);
    }, 3000);
    return () => window.clearTimeout(timer);
  }, [tipVisible, tipText]);

  return (
    <div
      className="min-h-screen"
      style={{
        background: 'var(--pc-bg-base)',
        color: 'var(--pc-text-primary)',
      }}
    >
      <LeftRail
        isAgentGenerating={isAgentGenerating}
        onBlockedNavigation={showGeneratingTip}
      />

      {/* Fixed sidebar */}
      <Sidebar open={sidebarOpen} onClose={() => setSidebarOpen(false)} />

      {/* Main area offset by sidebar width on desktop, full-width on mobile */}
      <div
        data-testid="layout-main-shell"
        data-nav-mode={currentMode.key}
        className={`ml-0 flex flex-col flex-1 min-w-0 h-screen ${desktopOffsetClass}`}
        style={isLight ? { background: '#FFFFFF' } : undefined}
      >
        <Header
          onMenuToggle={() => setSidebarOpen(true)}
          onNewAgentSession={() => handleBlockedOrRun(requestNewSession)}
        />

        {/* Page content — ErrorBoundary keyed by the first path segment so
            section-only route changes keep local page state. */}
        <main
          className="flex-1 overflow-y-auto min-h-0"
          style={isLight ? { background: '#FFFFFF' } : undefined}
        >
          <ErrorBoundary key={pathname.split('/')[1] ?? ''}>
            <Outlet context={{ setAgentGenerating: setIsAgentGenerating } satisfies AgentPageOutletContext} />
          </ErrorBoundary>
        </main>
      </div>
      {tipVisible && (
        <div className="pointer-events-none fixed inset-0 z-[70] flex items-center justify-center px-4">
          <div
            className="inline-flex min-h-[52px] w-fit max-w-[calc(100vw-2rem)] items-center gap-2 rounded-lg border px-4 py-3"
            style={{
              background: '#FFFFFF',
              borderColor: 'rgba(0, 0, 0, 0.1)',
              boxShadow: '0 4px 20px rgba(0, 0, 0, 0.2)',
            }}
            role="status"
            aria-live="polite"
          >
            <img
              src={`${assetBasePath}/icons/newbuttontips.svg`}
              alt=""
              className="h-4 w-4 shrink-0"
              aria-hidden="true"
            />
            <span className="text-sm" style={{ color: '#000000' }}>
              {tipText}
            </span>
          </div>
        </div>
      )}
    </div>
  );
}

export default function Layout() {
  return (
    <AgentNewSessionProvider>
      <LayoutShell />
    </AgentNewSessionProvider>
  );
}
