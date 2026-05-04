import type { Position } from '@/types/miniBoard'
import { createPosition } from '@/helpers/helpers'

function coordsOne(width: number, x: number, y: number): number {
  return x + y * width
}

export const positions: Position[] = [
  createPosition({
    width: 19,
    black: [],
    white: [],
    history: [141,121,106,102,83,140,159,123,105,122,124,104,85],
    label: 'cursed AI',
    title: 'The Ai got me in this positon',
    description: 'It\' an interesting positon, and also adding to to show that multiple positions can be listed here',
  }),
]