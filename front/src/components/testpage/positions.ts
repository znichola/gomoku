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
  createPosition({
    width: 19,
    black: [],
    white: [],
    history: [85,65,104,66,123,64,67,63,62,142,161,103,163,49,67,84,44,84,46,48,47,45,85,27,81,47,46,83,82,83,64,80,81,43,23,65,84,86,66],
    label: 'cursed AI',
    title: 'Something is off with it',
    description: 'I had saved this position as something odd happend with the ai, or how it managed a situation.',
  }),
]