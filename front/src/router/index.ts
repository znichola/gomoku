import Main from '@/components/Main.vue'
import Test from '@/components/testpage/Test.vue'
import { createRouter, createWebHistory } from 'vue-router'

const routes = [
  { path: "/", component: Main },
  { path: "/test", component: Test},
]

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes,
})

export default router
