const TOKEN_KEY = 'kylinbot_token';

function consumeTokenFromUrl(): void {
  if (typeof window === 'undefined') return;

  try {
    const url = new URL(window.location.href);
    const token = url.searchParams.get('token')?.trim();
    if (!token) return;

    localStorage.setItem(TOKEN_KEY, token);
    url.searchParams.delete('token');
    const cleanUrl = `${url.pathname}${url.search}${url.hash}`;
    window.history.replaceState(window.history.state, '', cleanUrl);
  } catch {
    // localStorage or history may be unavailable in restricted environments.
  }
}

consumeTokenFromUrl();

/**
 * Retrieve the stored authentication token.
 */
export function getToken(): string | null {
  try {
    return localStorage.getItem(TOKEN_KEY);
  } catch {
    return null;
  }
}

/**
 * Store an authentication token.
 */
export function setToken(token: string): void {
  try {
    localStorage.setItem(TOKEN_KEY, token);
  } catch {
    // localStorage may be unavailable (e.g. in some private browsing modes)
  }
}

/**
 * Remove the stored authentication token.
 */
export function clearToken(): void {
  try {
    localStorage.removeItem(TOKEN_KEY);
  } catch {
    // Ignore
  }
}

/**
 * Returns true if a token is currently stored.
 */
export function isAuthenticated(): boolean {
  const token = getToken();
  return token !== null && token.length > 0;
}
