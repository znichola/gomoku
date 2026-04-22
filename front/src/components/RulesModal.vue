<script setup lang="ts">
defineProps<{
  open: boolean
}>()

const emit = defineEmits<{
  (e: 'close'): void
}>()

function onBackdropClick(e: MouseEvent) {
  // Close only if the backdrop itself is clicked (not the modal content)
  if (e.target === e.currentTarget) emit('close')
}
</script>

<template>
  <teleport to="body">
    <div v-if="open" class="rm-backdrop" @click="onBackdropClick" aria-hidden="false">
      <div class="rm-modal" role="dialog" aria-modal="true" aria-label="Gomoku rules">
        <div class="rm-head">
          <h2 class="rm-title">Rules</h2>
          <button class="rm-close" type="button" @click="emit('close')" aria-label="Close">×</button>
        </div>

        <div class="rm-body">
          <h3>Goal</h3>
          <p>Two way to win:</p>
          <ul>
            <li>Be the first to align <strong>5 stones</strong> (horizontal, vertical, or diagonal).</li>
            <li>Be the first to captures <strong>10 stones</strong>.</li>
          </ul>

          <h3>Turn order</h3>
          <ul>
            <li>On your turn, place one stone on any empty intersection.</li>
            <li>Players alternate turns (Black / White).</li>
          </ul>

          <h3>Captures</h3>
          <ul>
            <li>If you surround <strong>two opponent stones</strong> with your stones (pattern <code>X O O X</code>), you capture them.</li>
            <li>Captures are shown in the HUD (bowls).</li>
          </ul>
        </div>

        <div class="rm-foot">
          <button class="rm-btn" type="button" @click="emit('close')">OK</button>
        </div>
      </div>
    </div>
  </teleport>
</template>

<style scoped lang="less">
.rm-backdrop {
  position: fixed;
  inset: 0;
  background: rgba(0, 0, 0, 0.55);
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 1rem;
  z-index: 9999;
}

.rm-modal {
  width: min(700px, 92vw);
  border: solid 0.165rem var(--accent-color);
  background-color: var(--bg-color);
  border-radius: 1.2rem;
  color: var(--primary-color);
  box-shadow: 0 20px 60px rgba(0, 0, 0, 0.45);
}

.rm-head {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 1rem;
  padding: 0.8rem 0.9rem 0.6rem;
  border-bottom: solid 1px var(--line-color);
}

.rm-title {
  font-family: var(--title-font-family);
  font-weight: 400;
  font-size: 2.2rem;
  color: var(--accent-color);
  line-height: 1;
}

.rm-close {
  width: 2.4rem;
  height: 2.4rem;
  border-radius: 0.8rem;
  background: transparent;
  border: solid 1px var(--line-color);
  color: var(--primary-color);
  font: var(--ui-font);
  font-size: 1.4rem;
  cursor: pointer;
}

.rm-close:hover {
  border-color: var(--accent-color);
  color: var(--accent-color);
}

.rm-body {
  padding: 0.8rem 0.9rem;
  font-family: var(--ui-font-family);
}

.rm-body h3 {
  font: var(--ui-font);
  font-size: 1.1rem;
  margin: 0.8rem 0 0.35rem;
}

.rm-body h3:first-child {
  margin-top: 0;
}

.rm-body ul {
  display: flex;
  flex-direction: column;
  gap: 0.35rem;
}

.rm-body code {
  color: var(--highlight-color);
  font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace;
  font-size: 0.95em;
}

.rm-foot {
  display: flex;
  justify-content: flex-end;
  padding: 0.7rem 0.9rem 0.9rem;
  border-top: solid 1px var(--line-color);
}

.rm-btn {
  border: solid 0.165rem var(--accent-color);
  background-color: transparent;
  color: var(--primary-color);
  border-radius: 0.9rem;
  padding: 0.35rem 0.8rem;
  font: var(--ui-font);
  cursor: pointer;
}

.rm-btn:hover {
  background-color: rgba(211, 80, 19, 0.12);
}

p {
  line-height: 1.4rem;
}

ul {
  list-style-type: disc;
  padding-left: 20px;
}
</style>
