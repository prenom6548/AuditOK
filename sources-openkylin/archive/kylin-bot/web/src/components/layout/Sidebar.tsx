import { NavLink, useLocation } from 'react-router-dom';
import {
  Database,
  Sparkles,
  Settings,
  BarChart2,
} from 'lucide-react';
import { t } from '@/lib/i18n';
import { resolveNavMode, type NavItemPath } from './navModes';
import { useTheme } from '@/hooks/useTheme';

const navItems = [
  { to: '/general', icon: Settings, labelKey: 'nav.general' },
  { to: '/skills', icon: Sparkles, labelKey: 'nav.skills' },
  { to: '/cost', icon: BarChart2, labelKey: 'nav.cost' },
  { to: '/memory', icon: Database, labelKey: 'nav.memory' },
] as const satisfies ReadonlyArray<{ to: NavItemPath; icon: React.ComponentType<{ className?: string }>; labelKey: string }>;

interface SidebarProps {
  open: boolean;
  onClose: () => void;
}

export default function Sidebar({ open, onClose }: SidebarProps) {
  const { pathname } = useLocation();
  const currentMode = resolveNavMode(pathname);
  const visibleNavItems = navItems.filter(({ to }) => currentMode.visibleNavItems.includes(to));
  const { resolvedTheme } = useTheme();
  const isLight = resolvedTheme === 'light';
  const desktopVisible = currentMode.key !== 'agent';

  return (
    <>
      {/* Backdrop — mobile only, visible when sidebar is open */}
      {open && (
        <div
          className="md:hidden fixed inset-0 z-40 bg-black/60 backdrop-blur-sm transition-opacity"
          onClick={onClose}
          onKeyDown={(e) => { if (e.key === 'Escape') onClose(); }}
          role="button"
          tabIndex={-1}
          aria-label="Close menu"
        />
      )}

      <aside
        data-testid="app-sidebar"
        data-light-theme={isLight ? 'true' : 'false'}
        data-desktop-visible={desktopVisible ? 'true' : 'false'}
        className={[
          'fixed top-0 left-0 h-screen w-60 flex flex-col z-30',
          desktopVisible ? 'md:left-14 md:flex' : 'md:hidden',
          // Mobile: slide in/out with transition
          'max-md:-translate-x-full max-md:transition-transform max-md:duration-200 max-md:ease-out',
          open ? 'max-md:translate-x-0' : '',
        ].join(' ')}
        style={{
          background: '#FFFFFF',
          boxShadow: 'none',
        }}
      >
        <div
          className="hidden md:block h-10 border-b"
          style={{ background: '#FFFFFF', borderColor: '#E6E6E6' }}
          aria-hidden="true"
        />
        {/* Navigation */}
        <div className="flex-1 border-r" style={{ borderColor: '#E6E6E6' }}>
          <nav className="h-full overflow-y-auto py-4 px-3 space-y-1">
            {visibleNavItems.map(({ to, icon: Icon, labelKey }, idx) => (
              <NavLink
                key={to}
                to={to}
                end
                onClick={onClose}
                className={({ isActive }) =>
                  [
                    'flex items-center gap-3 h-9 px-3 rounded-[6px] text-sm font-medium transition-colors group',
                    isActive
                      ? 'text-white'
                      : 'text-[#111827] hover:text-white',
                  ].join(' ')
                }
                style={({ isActive }) => ({
                  animationDelay: `${idx * 40}ms`,
                  ...(isActive ? {
                    background: '#3790FA',
                    border: '1px solid #3790FA',
                    boxShadow: 'none',
                  } : {
                    background: 'transparent',
                    border: '1px solid transparent',
                  }),
                })}
                onMouseEnter={(e) => {
                  if (!e.currentTarget.getAttribute('aria-current')) {
                    e.currentTarget.style.background = '#3790FA';
                  }
                }}
                onMouseLeave={(e) => {
                  if (!e.currentTarget.getAttribute('aria-current')) {
                    e.currentTarget.style.background = 'transparent';
                  }
                }}
              >
                {({ isActive }) => (
                  <>
                    <Icon className={`h-4 w-4 flex-shrink-0 transition-colors ${isActive ? 'text-white' : 'text-[#111827] group-hover:text-white'}`} />
                    <span>{t(labelKey)}</span>
                  </>
                )}
              </NavLink>
            ))}
          </nav>
        </div>


      </aside>
    </>
  );
}
