import { Suspense } from 'react';
import { Navigate, Route, Routes } from 'react-router-dom';
import Layout from '../components/layout/Layout';
import {
  AgentChat,
  AgentHistory,
  Canvas,
  Config,
  Cost,
  Cron,
  Dashboard,
  Doctor,
  General,
  Integrations,
  Logs,
  Memory,
  Onboard,
  Pairing,
  Skills,
  Tools,
} from './lazyPages';

function RouteFallback() {
  return (
    <div className="min-h-[60vh] flex items-center justify-center">
      <div
        className="h-8 w-8 border-2 rounded-full animate-spin"
        style={{ borderColor: 'var(--pc-border)', borderTopColor: 'var(--pc-accent)' }}
      />
    </div>
  );
}

export const Router = () => (
  <Suspense fallback={<RouteFallback />}>
    <Routes>
      <Route element={<Layout />}>
        <Route path="/" element={<Dashboard />} />
        <Route path="/agent" element={<AgentChat />} />
        <Route path="/agent/history" element={<AgentHistory />} />
        <Route path="/tools" element={<Tools />} />
        <Route path="/cron" element={<Cron />} />
        <Route path="/integrations" element={<Integrations />} />
        <Route path="/general" element={<General />} />
        <Route path="/skills" element={<Skills />} />
        <Route path="/memory" element={<Memory />} />
        <Route path="/config" element={<Config />} />
        <Route path="/config/:section" element={<Config />} />
        <Route path="/setup/:section" element={<Config />} />
        <Route path="/cost" element={<Cost />} />
        <Route path="/logs" element={<Logs />} />
        <Route path="/doctor" element={<Doctor />} />
        <Route path="/pairing" element={<Pairing />} />
        <Route path="/canvas" element={<Canvas />} />
        <Route path="/onboard" element={<Onboard />} />
        <Route path="*" element={<Navigate to="/agent" replace />} />
      </Route>
    </Routes>
  </Suspense>
);
