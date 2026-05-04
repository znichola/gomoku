<template>
  <div class="not-found">
    <h1>404</h1>
    <router-link to="/">Go back home</router-link>
  </div>
</template>

<style scoped>
.not-found {
  text-align: center;
  margin-top: 6rem;
}
h1 {
  font-size: 4rem;
}

a {
  color: var(--primary-color);
}

a:hover {
  color: var(--accent-color);
}
</style>