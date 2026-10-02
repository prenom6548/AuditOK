import { render, screen } from '@testing-library/react';
import { describe, expect, it } from 'vitest';
import { MemoryRouter } from 'react-router-dom';
import Sidebar from './Sidebar';
import { ThemeContext, type ThemeContextValue } from '@/contexts/ThemeContextDef';

function renderLightSidebar(initialEntries = ['/agent']) {
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
      <MemoryRouter initialEntries={initialEntries}>
        <Sidebar open={false} onClose={() => {}} />
      </MemoryRouter>
    </ThemeContext.Provider>,
  );
}

describe('Sidebar', () => {
  it('renders no configuration nav links while on the agent route (drawer is contextual)', () => {
    renderLightSidebar(['/agent']);

    expect(screen.queryByRole('link', { name: 'General' })).not.toBeInTheDocument();
    expect(screen.getByTestId('app-sidebar')).toHaveAttribute('data-desktop-visible', 'false');
  });

  it('shows configuration nav links when browsing general settings', () => {
    renderLightSidebar(['/general']);

    expect(screen.getByRole('link', { name: 'General' })).toBeInTheDocument();
    expect(screen.getByRole('link', { name: 'Skills' })).toBeInTheDocument();
    expect(screen.getByTestId('app-sidebar')).toHaveAttribute('data-desktop-visible', 'true');
  });

  it('marks the light-theme sidebar for the reference window styling', () => {
    renderLightSidebar(['/agent']);

    expect(screen.getByTestId('app-sidebar')).toHaveAttribute('data-light-theme', 'true');
  });
});
