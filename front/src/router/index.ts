import Main from '@/components/Main.vue'
import PositionPlayground from '@/components/testpage/PositionPlayground.vue'
import NotFound from '@/components/NotFound.vue'
import { createRouter, createWebHistory } from 'vue-router'

const routes = [
  { path: "/", component: Main },
  { path: "/pp", component: PositionPlayground},
  { path: "/:pathMatch(.*)*", name: "NotFound", component: NotFound },
]

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes,
})

export default router
