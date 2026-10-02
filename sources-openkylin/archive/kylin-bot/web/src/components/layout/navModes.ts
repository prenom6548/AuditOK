export type NavItemPath =
  | '/agent'
  | '/agent/history'
  | '/general'
  | '/skills'
  | '/cost'
  | '/memory';

type NavMode = {
  key: 'agent' | 'config';
  railTo: NavItemPath;
  matches: (pathname: string) => boolean;
  visibleNavItems: NavItemPath[];
};

const configNavItems: NavItemPath[] = [
  '/agent',
  '/general',
  '/skills',
  '/cost',
  '/memory',
];

export const navModes: NavMode[] = [
  {
    key: 'agent',
    railTo: '/agent',
    matches: (pathname) => pathname === '/agent' || pathname === '/agent/history',
    visibleNavItems: ['/agent', '/agent/history'],
  },
  {
    key: 'config',
    railTo: '/general',
    matches: (pathname) => configNavItems.includes(pathname as NavItemPath),
    visibleNavItems: ['/general', '/skills', '/cost', '/memory'],
  },
];

export function resolveNavMode(pathname: string): NavMode {
  return navModes.find((mode) => mode.matches(pathname)) ?? navModes[0]!;
}
