// Backend base URL. Configurable at build time via VITE_API_BASE_URL (e.g. to
// point a dev frontend at a backend on another host/port, or a deployment
// where the frontend isn't served by the same backend). When unset:
//  - dev (`vite`/`npm run dev`): talk to the local watcher backend directly,
//    using whatever hostname the page was loaded from (so LAN/phone access
//    during dev still works).
//  - build (`vite build`): '' — relative to the current origin, since the
//    production backend serves both the static frontend and the API.
function apiBase(): string {
  const configured = import.meta.env.VITE_API_BASE_URL
  if (configured) return configured.replace(/\/$/, '')
  if (import.meta.env.DEV) return `http://${window.location.hostname}:9012`
  return ''
}

// Builds a URL for an /api/* backend route, e.g. apiUrl('/gameState') or
// apiUrl('gameState') -> 'http://host:9012/api/gameState' in dev, '/api/gameState' in prod.
export function apiUrl(path: string): string {
  const normalized = path.startsWith('/') ? path : `/${path}`
  return `${apiBase()}/api${normalized}`
}
