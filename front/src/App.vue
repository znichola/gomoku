<script setup lang="ts">
import '../assets/styles.css'

</script>

<template>
  <main>
    <h1>Gomoku</h1>
    <!-- Page Router view -->
    <router-view />
  </main>
</template>

<style lang="less">
body {
  --bg-color: #2d3442;
  --line-color: #676a79;
  --text-color: var(--bg-color);
  --accent-color: #d35013;
  --black-color: var(--accent-color);
  --white-color: #ffffff;
}

main {
  width: 100vw;
  min-height: 100vh;
  overflow: hidden;
  display: flex;
  flex-direction: column;
  align-items: center;
  background-color: var(--bg-color);
}

h1 {
  font-family: "Pirata One", system-ui;
  font-weight: 400;
  font-style: normal;
  font-size: 5rem;
  color: #d35013;
}
</style>
