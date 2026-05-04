<script setup lang="ts">
import { computed, ref } from 'vue'
import BoardPositionsSection from '@/components/BoardPositionsSection.vue'
import { positions } from './positions'

</script>

<template>
  <div class="gallery">
    <BoardPositionsSection
      v-for="pos in positions"
      :key="pos.id"
      v-bind="pos"
    >
    </BoardPositionsSection>
  </div>
</template>

<style scoped>
.gallery {
  max-width: 1100px;
  margin: 0 auto;
}
</style>