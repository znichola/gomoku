import { Cell } from '@/types/game';

function getCellClass(cell: Cell): string {
  switch (cell) {
    case Cell.BLACK:
      return 'black';
    case Cell.WHITE:
      return 'white';
    default:
      return '';
  }
}

export { getCellClass };