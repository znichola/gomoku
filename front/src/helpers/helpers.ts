import { Cell } from '@/types/game';

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

const searchOptions: { value: string; label: string; desc: string }[] = [
  { value: 'MINMAX',               label: 'Minimax',         desc: 'Basic two-player search' },
  { value: 'MINMAX_JETESTE',       label: 'Minimax (G)',     desc: 'Basic two-player search' },
  { value: 'NEGAMAX',              label: 'Negamax',         desc: 'Simplified minimax variant' },
  { value: 'ALPHABETA_NEGAMAX',    label: 'Alpha-beta',      desc: 'Negamax with alpha-beta pruning' },
  { value: 'ALPHABETA_NEGAMAX_TT', label: 'Alpha-beta + TT', desc: 'Negamax with alpha-beta pruning and a transposition table' },
]

export { getCellClass, searchOptions };
