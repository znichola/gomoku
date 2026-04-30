import { defineStore } from 'pinia'
import { ref } from 'vue'

type Job = () => Promise<void>

export const usePositionQueueStore = defineStore('positionQueue', () => {
  const pending = ref(0)
  const running = ref(false)

  const _queue: Job[] = []

  async function _drain() {
    if (running.value) return
    running.value = true

    while (_queue.length) {
      const job = _queue.shift()!
      pending.value = _queue.length
      await job().catch(() => { /* each job manages its own error state */ })
    }

    running.value = false
    pending.value = 0
  }

  function enqueue(job: Job) {
    _queue.push(job)
    pending.value = _queue.length
    _drain()
  }

  return { pending, running, enqueue }
})