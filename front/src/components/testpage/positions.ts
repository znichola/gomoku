import type { Position } from '@/types/miniBoard'

function coordsOne(width: number, x: number, y: number): number {
  return x + y * width
}

export const positions: Position[] = [
  {
    id: 'strange-win',
    width: 19,
    black: [],
    white: [],
    history: [85,65,104,66,123,64,67,63,62,142,161,103,163,49,67,84,44,84,46,48,47,45,85,27,81,47,46,83,82,83,64,80,81,43,23,65,84,86,66],
    // label: 'strange win',
    title: 'Not sure why it\'s a win ?',
    description: 'Got this position randomly after doing play, undo etc. and it shows as a win, but idk',
  },
  {
    id: 'strange-win',
    width: 19,
    black: [],
    white: [],
    history: [141,121,106,102,83,140,159,123,105,122,124,104,85],
    label: 'cursed AI',
    title: 'The Ai got me in this positon',
    description: 'It\' an interesting positon, and also adding to to show that multiple positions can be listed here',
  },
]