<script setup lang="ts">
defineProps<{
  active?: boolean
  disabled?: boolean
  variant?: 'default' | 'accent' | 'danger'
}>()
</script>
<template>
  <button class="app-btn" :class="[$props.variant ?? 'default', { active }]" :disabled="disabled" type="button">
    <slot></slot>
  </button>
</template>

<style scoped lang="less">
.app-btn {
  cursor: pointer;
  display: inline-block;
  padding: 0.2rem 0.6rem;
  border: solid 0.165rem var(--accent-color);
  border-radius: 1.2rem;
  background-color: var(--primary-color);
  color: var(--bg-color);
  font: var(--ui-font);
  transition: all 0.3s ease;
  line-height: 1.4;
  white-space: nowrap;

  &:hover, &.active {
    background-color: var(--accent-color);
    color: var(--primary-color);
  }
  
  &.active:hover {
    background-color: var(--primary-color);
    color: var(--bg-color);
  }
  
  &.active {
    transform: scale(0.90);
    filter: brightness(1.1);
  }

  &.accent {
    background-color: var(--accent-color);
    color: var(--primary-color);

    &:hover {
      background-color: var(--primary-color);
      color: var(--bg-color);
    }
  }

  &.danger {
    border-color: indianred;
    &:hover, &.active {
      background-color: indianred;
      color: white;
    }
  }

  &:disabled {
    cursor: not-allowed;
    opacity: 0.5;

    /* lock visuals */
    background-color: var(--primary-color);
    color: var(--bg-color);
    border-color: var(--accent-color);

    /* kill interaction completely */
    pointer-events: none;
    transform: none;
  }

  /* optional: remove transitions when disabled */
  &:disabled {
    transition: none;
  }
}
</style>