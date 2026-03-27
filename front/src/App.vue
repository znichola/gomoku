<script setup lang="ts">
import '../assets/styles.css'

</script>

<template>
  <main>
    <!-- Page Router view -->
    <router-view />
  </main>
</template>

<style lang="less">
html {
  font-size: clamp(0.7rem, 1vw + 0.5rem, 1.2rem);
}

body {
  --primary-color: #e2d9b2;
  --accent-color: #d35013;
  --bg-color: #2d3442;
  --line-color: #676a79;
  --text-color: var(--bg-color);
  --black-color: var(--accent-color);
  --white-color: var(--primary-color);
  --title-font-family: 'Pirata One', 'Impact', 'Arial Narrow Bold', sans-serif;
  --ui-font-family: 'Galdeano', sans-serif;
  --ui-font: bold 1.4rem var(--ui-font-family);
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
  font-family: var(--title-font-family);
  font-weight: 400;
  font-style: normal;
  font-size: 5rem;
  color: #d35013;
}
</style>
