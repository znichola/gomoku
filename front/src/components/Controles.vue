<script setup lang="ts">

async function reset() {
  try {
    const resp = await fetch(`http://localhost:9012/reset`, {
      method: 'GET',
    })
    const data = await resp.json()
    console.log(data)
    } catch (err) {
    console.warn(err)
  }
}

</script>

<template>
<button class="reset-btn" @click="$emit('reset')">Restart</button>
</template>

<style scoped lang="scss">

</style>