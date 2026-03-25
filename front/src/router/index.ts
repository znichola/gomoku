import Game from '@/components/Game.vue'
import { createRouter, createWebHistory } from 'vue-router'

const routes = [
  { path: "/", component: Game },
]

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes,
})

export default router
