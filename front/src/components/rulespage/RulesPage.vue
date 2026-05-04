<script setup lang="ts">
import PositionRules from './PositionRules.vue'
</script>

<template>
  <div class="tpage">
    <header class="tpage-header">
      <h1 class="tpage-title"><router-link to="/">↩</router-link>Rules</h1>
      <p class="tpage-sub">Loosly based of the rules for Pent, and Gomoku</p>
    </header>
    <PositionRules />
  </div>
</template>

<style scoped >
.tpage {
  min-height: 100vh;
  color: var(--primary-color);
  padding: 2rem 1.5rem 4rem;
  box-sizing: border-box;
  font-family: var(--ui-font-family);
}

.tpage-header {
  text-align: center;
  margin-bottom: 2.5rem;

  a {
    color: inherit;
    text-decoration: none;
    font-size: x-large;
    color: var(--primary-color);
    padding-right: 0.3rem;
  }

  a:hover {
    color: var(--accent-color);
  }
}


.tpage-title {
  font-family: var(--title-font-family);
  font-size: clamp(2.8rem, 6vw, 5rem);
  font-weight: 400;
  color: var(--accent-color);
  margin: 0 0 0.2rem;
  letter-spacing: 0.02em;
}

.tpage-sub {
  font-family: var(--ui-font);
  font-size: 0.95rem;
  color: var(--line-color);
  margin: 0;
  letter-spacing: 0.06em;
  text-transform: uppercase;
}
</style>