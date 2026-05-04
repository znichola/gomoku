import type { Position } from '@/types/miniBoard'
import { createPosition } from '@/helpers/helpers'

function coordsOne(width: number, x: number, y: number): number {
  return x + y * width
}

// 158,138,125,119,100,157,176,140,194,139,141,121,159,160,159,120,102,121,99,119,101,123

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
    history: [158,138,125,119,100,157,176,140,194,139,141,121,159,160,159,120,102,121,99,119,101,123],
    label: 'cursed AI',
    title: 'I beat the AI here',
    description: 'Looks like the AI did not want to take the winnning move when it had the options.<br> Evaluate for some reason returns higher than win scores.',
  }),

  createPosition({
    width: 19,
    black: [],
    white: [],
    history: [180,125,160,140,200,220,199,218,198,216,179,217,219,161,199,238,202,181,219],
    label: 'cursed AI',
    title: 'Again a straightforward win',
    description: 'AI was maxed out at 10 depth, but it can be pushed around a bit.',
  }),
]