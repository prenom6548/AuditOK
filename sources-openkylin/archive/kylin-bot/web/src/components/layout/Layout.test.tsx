import { render, screen } from '@testing-library/react';
import { MemoryRouter, Route, Routes } from 'react-router-dom';
import { describe, expect, it, vi } from 'vitest';
import Layout from './Layout';
import { ThemeContext, type ThemeContextValue } from '@/contexts/ThemeContextDef';

vi.mock('@/components/layout/Header', () => ({
  default: ({ onMenuToggle }: { onMenuToggle: () => void }) => (
    <button type="button" onClick={onMenuToggle}>header</button>
  ),
}));

vi.mock('@/App', () => ({
  ErrorBoundary: ({ children }: { children: React.ReactNode }) => <>{children}</>,
}));

function renderLayout(initialEntry: string) {
  const themeValue: ThemeContextValue = {
    theme: 'light',
    accent: 'blue',
    colorTheme: 'default-light',
    uiFont: 'system',
    monoFont: 'jetbrains',
    uiFontSize: 15,
    monoFontSize: 14,
    resolvedTheme: 'light',
    setTheme: () => {},
    setAccent: () => {},
    setColorTheme: () => {},
    setUiFont: () => {},
    setMonoFont: () => {},
    setUiFontSize: () => {},
    setMonoFontSize: () => {},
  };

  return render(
    <ThemeContext.Provider value={themeValue}>
      <MemoryRouter initialEntries={[initialEntry]}>
        <Routes>
          <Route element={<Layout />}>
            <Route path="/agent" element={<div>agent page</div>} />
            <Route path="/general" element={<div>general page</div>} />
          </Route>
        </Routes>
      </MemoryRouter>
    </ThemeContext.Provider>,
  );
}

describe('Layout', () => {
  it('uses the compact desktop offset on the agent route', () => {
    renderLayout('/agent');

    expect(screen.getByTestId('layout-main-shell')).toHaveAttribute('data-nav-mode', 'agent');
    expect(screen.getByTestId('app-sidebar')).toHaveAttribute('data-desktop-visible', 'false');
  });

  it('uses the expanded desktop offset on config-mode routes', () => {
    renderLayout('/general');

    expect(screen.getByTestId('layout-main-shell')).toHaveAttribute('data-nav-mode', 'config');
    expect(screen.getByTestId('app-sidebar')).toHaveAttribute('data-desktop-visible', 'true');
  });
});
