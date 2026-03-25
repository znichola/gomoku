import type { Ref } from "vue";

export type RefNull = Ref<null, null>
export type RefStringOrNull = Ref<String, String> | RefNull
