import { render, screen } from '@testing-library/react';
import { MemoryRouter } from 'react-router-dom';
import { describe, expect, it } from 'vitest';
import LeftRail from './LeftRail';
import { ThemeContext, type ThemeContextValue } from '@/contexts/ThemeContextDef';

function renderLightRail(initialEntries = ['/agent']) {
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
        <LeftRail />
      </MemoryRouter>
    </ThemeContext.Provider>,
  );
}

const SELECTED_LIGHT_BG = 'rgb(55, 144, 250)';

describe('LeftRail', () => {
  it('renders quick navigation buttons for agent chat and config', () => {
    renderLightRail(['/agent']);

    expect(screen.getByRole('link', { name: 'History' })).toHaveAttribute('href', '/agent/history');
    expect(screen.getByRole('link', { name: 'Agent' })).toHaveAttribute('href', '/agent');
    expect(screen.getByRole('link', { name: 'General' })).toHaveAttribute('href', '/general');
  });

  it('renders quick navigation buttons for agent chat and config', () => {
    renderLightRail(['/agent']);

    expect(screen.getByRole('link', { name: 'Agent' })).toHaveStyle({
      background: SELECTED_LIGHT_BG,
    });
    expect(screen.getByRole('link', { name: 'General' })).not.toHaveStyle({
      background: SELECTED_LIGHT_BG,
    });
  });

  it('selects the config button on non-agent routes', () => {
    renderLightRail(['/general']);

    expect(screen.getByRole('link', { name: 'Agent' })).not.toHaveStyle({
      background: SELECTED_LIGHT_BG,
    });
    expect(screen.getByRole('link', { name: 'General' })).toHaveStyle({
      background: SELECTED_LIGHT_BG,
    });
  });

  it('selects the config button for config-mode routes', () => {
    renderLightRail(['/skills']);

    expect(screen.getByRole('link', { name: 'Agent' })).not.toHaveStyle({
      background: SELECTED_LIGHT_BG,
    });
    expect(screen.getByRole('link', { name: 'General' })).toHaveStyle({
      background: SELECTED_LIGHT_BG,
    });
  });

  it('selects history button and unselects chat on history route', () => {
    renderLightRail(['/agent/history']);

    expect(screen.getByRole('link', { name: 'Agent' })).not.toHaveStyle({
      background: SELECTED_LIGHT_BG,
    });
    expect(screen.getByRole('link', { name: 'History' })).toHaveStyle({
      background: SELECTED_LIGHT_BG,
    });
  });

  it('marks the light-theme rail for the compact reference layout', () => {
    renderLightRail(['/agent']);

    expect(screen.getByTestId('left-rail')).toHaveAttribute('data-light-theme', 'true');
  });
});
