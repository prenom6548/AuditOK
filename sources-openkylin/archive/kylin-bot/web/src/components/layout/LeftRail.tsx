import { NavLink, useLocation } from 'react-router-dom';
import { MessageSquare, Settings } from 'lucide-react';
import type { LucideIcon } from 'lucide-react';
import { t } from '@/lib/i18n';
import { assetBasePath } from '@/lib/basePath';
import { resolveNavMode } from './navModes';
import { useTheme } from '@/hooks/useTheme';

const railItems = [
  { to: '/agent', icon: MessageSquare, labelKey: 'nav.agent', assetIcon: 'qa.svg' },
  { to: '/agent/history', icon: MessageSquare, labelKey: 'nav.history', assetIcon: 'history.svg' },
  { to: '/general', icon: Settings, labelKey: 'nav.general', assetIcon: 'application-system-symbolic.svg' },
] as const;

interface LeftRailProps {
  isAgentGenerating?: boolean;
  onBlockedNavigation?: () => void;
}

export default function LeftRail({
  isAgentGenerating = false,
  onBlockedNavigation,
}: LeftRailProps) {
  const { pathname } = useLocation();
  const [chatItem, historyItem, configItem] = railItems;
  const currentMode = resolveNavMode(pathname);
  const { resolvedTheme } = useTheme();
  const isLight = resolvedTheme === 'light';

  return (
    <aside
      data-testid="left-rail"
      data-light-theme={isLight ? 'true' : 'false'}
      className="hidden md:flex fixed top-0 left-0 z-40 h-screen w-14 flex-col justify-between border-r px-2 py-2"
      style={{
        background: isLight ? 'var(--pc-bg-sidebar)' : 'var(--pc-bg-surface)',
        borderColor: 'var(--pc-border)',
        boxShadow: isLight ? 'inset -1px 0 0 rgba(255,255,255,0.7)' : 'none',
      }}
      aria-label="Quick navigation"
    >
      <div className={`flex flex-col items-center ${isLight ? 'gap-[10px] pt-1' : 'gap-2'}`}>
        <div className="flex h-9 w-9 shrink-0 items-center justify-center rounded-lg">
          <img
            src={`${assetBasePath}/kylin-bot.svg`}
            alt="KylinBot"
            className="h-6 w-6 rounded-md object-cover"
            onError={(e) => {
              const img = e.currentTarget;
              img.style.display = 'none';
            }}
          />
        </div>
        <RailButton
          {...chatItem}
          selected={pathname === chatItem.to}
          isLight={isLight}
          onBlockedNavigation={onBlockedNavigation}
          isAgentGenerating={isAgentGenerating}
        />
      </div>
      <div className={`flex flex-col items-center ${isLight ? 'gap-[10px]' : 'gap-2'}`}>
        <RailButton
          {...historyItem}
          selected={pathname === '/agent/history'}
          isLight={isLight}
          onBlockedNavigation={onBlockedNavigation}
          isAgentGenerating={isAgentGenerating}
        />
        <RailButton
          {...configItem}
          selected={currentMode.railTo === configItem.to}
          isLight={isLight}
          onBlockedNavigation={onBlockedNavigation}
          isAgentGenerating={isAgentGenerating}
        />
      </div>
    </aside>
  );
}

function RailButton({
  to,
  icon: Icon,
  labelKey,
  selected,
  assetIcon,
  isLight,
  isAgentGenerating,
  onBlockedNavigation,
}: {
  to: string;
  icon: LucideIcon;
  labelKey: string;
  selected: boolean;
  assetIcon?: string;
  isLight: boolean;
  isAgentGenerating: boolean;
  onBlockedNavigation?: () => void;
}) {
  const label = t(labelKey);
  const selectedBackground = isLight ? '#3790FA' : 'var(--pc-accent-glow)';
  const hoverClass = isLight ? 'hover:bg-[rgba(0,0,0,0.05)]' : 'hover:bg-[var(--pc-bg-elevated)]';
  const activeClass = isLight ? 'active:bg-[#3790FA]' : 'active:bg-[var(--pc-bg-elevated)]';

  return (
    <NavLink
      to={to}
      className={[
        'flex h-9 w-9 shrink-0 items-center justify-center rounded-lg transition-colors select-none',
        selected ? 'text-white' : `text-[rgba(0,0,0,0.85)] ${hoverClass} ${activeClass} active:text-white`,
      ].join(' ')}
      style={{
        ...(selected ? { background: selectedBackground } : {}),
      }}
      aria-label={label}
      title={label}
      onClick={(e) => {
        if (isAgentGenerating && !selected) {
          e.preventDefault();
          onBlockedNavigation?.();
        }
      }}
    >
      {assetIcon ? (
        <span
          aria-hidden="true"
          className="block h-4 w-4"
          style={{
            backgroundColor: 'currentColor',
            WebkitMaskImage: `url(${assetBasePath}/icons/${assetIcon})`,
            WebkitMaskRepeat: 'no-repeat',
            WebkitMaskPosition: 'center',
            WebkitMaskSize: 'contain',
            maskImage: `url(${assetBasePath}/icons/${assetIcon})`,
            maskRepeat: 'no-repeat',
            maskPosition: 'center',
            maskSize: 'contain',
          }}
        />
      ) : (
        <Icon className="h-4 w-4" strokeWidth={1.9} />
      )}
    </NavLink>
  );
}
