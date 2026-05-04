import { Cell } from '@/types/game'
import type { SearchFunction, MoveFunction } from '@/types/game'
import type { Position } from '@/types/miniBoard'

function getCellClass(cell: Cell): string {
  switch (cell) {
    case Cell.BLACK:
      return 'black';
    case Cell.WHITE:
      return 'white';
    default:
      return 'empty';
  }
}

const searchOptions: { value: SearchFunction; label: string; desc: string }[] = [
  { value: 'MINMAX',               label: 'Minimax',         desc: 'Basic two-player search' },
  { value: 'MINMAX_JETESTE',       label: 'Minimax (G)',     desc: 'Basic two-player search' },
  { value: 'NEGAMAX',              label: 'Negamax',         desc: 'Simplified minimax variant' },
  { value: 'ALPHABETA_NEGAMAX',    label: 'Alpha-beta',      desc: 'Negamax with alpha-beta pruning' },
  { value: 'ALPHABETA_NEGAMAX_TT', label: 'Alpha-beta + TT', desc: 'Negamax with alpha-beta pruning and a transposition table' },
]

const moveOptions: { value: MoveFunction; label: string; desc: string }[] = [
  { value: 'CANDIDATE_MOVES',      label: 'Candidate moves', desc: 'Basic un ordered moves selection' },
  { value: 'CANDIDATE_MOVES_2',    label: 'Candidate moves 2', desc: 'Ordering based on evaluation delta after move, and aggressive move pruning' },
  { value: 'JETEST',               label: 'Candidate moves (G)', desc: 'Attempted ordering and restricting of possible moves' },
]

export { getCellClass, searchOptions, moveOptions };


// PostionStats Helpers

export function coords(width: number, ...pairs: [number, number][]): number[] {
  return pairs.map(([x, y]) => x + y * width)
}

export function coordsOne(width: number, x: number, y: number): number {
  return x + y * width
}

let positionCounter = 0

export type PositionInput = Omit<Position, 'id'> & { id?: string }

export function createPosition(pos: PositionInput): Position {
  return {
    ...pos,
    id: pos.id ?? `pos-${++positionCounter}`
  }
}
