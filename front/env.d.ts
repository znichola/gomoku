/// <reference types="vite/client" />

interface ImportMetaEnv {
  // Backend base URL override, see src/helpers/api.ts. Optional: leave unset
  // to get sensible dev/prod defaults.
  readonly VITE_API_BASE_URL?: string
}

interface ImportMeta {
  readonly env: ImportMetaEnv
}
