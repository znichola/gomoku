import Main from '@/components/Main.vue'
import { createRouter, createWebHistory } from 'vue-router'

const routes = [
  { path: "/", component: Main },
]

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes,
})

export default router
