import type { Position } from '@/types/miniBoard'

function coords(width: number, ...pairs: [number, number][]): number[] {
  return pairs.map(([x, y]) => x + y * width)
}

function coordsOne(width: number, x: number, y: number): number {
  return x + y * width
}

export const positions: Position[] = [
  {
    id: 'capture-threat',
    width: 7,
    black: coords(7, [2, 3], [3, 3]),
    white: coords(7, [1, 3], [4, 3]),
    highlight: [
      { id: coordsOne(7, 4, 3), label: 'W' },
    ],
    label: '7 × 7 · capture',
    title: 'Capture — flanking a pair',
    description:
      'White has just bracketed the two black stones with moves at <strong>W</strong>. ' +
      'In Pente, any pair flanked on both ends is immediately removed. ' +
      'Five such captures (10 stones total) is an alternate win condition, ' +
      'so piling captures is a legitimate strategy alongside five-in-a-row.',
  },

  {
    id: 'open-four',
    width: 9,
    black: coords(9, [2, 4], [3, 4], [4, 4], [5, 4]),
    white: coords(9, [2, 2], [6, 3], [1, 5]),
    highlight: [{ id: coordsOne(9, 6, 4), highlight: true }, { id: coordsOne(9, 1, 4), highlight: true }],
    label: '9 × 9 · four',
    title: 'Open four — one from five',
    description:
      'Black has four in a row with <em>both</em> ends open (an "open four" or <em>tessara</em>). ' +
      'White cannot block both ends in one move, so Black wins next turn. ' +
      'The teal rings marks the winning extension. ' +
      'Creating an open four is the core tactical goal of mid-game play.',
  },

  {
    id: 'double-threat',
    width: 9,
    black: coords(9,
      [2, 6], [3, 5], [4, 4], [5, 3],   // diagonal four
      [4, 2], [4, 3],                     // vertical support
      [2, 4], [3, 4],                     // horizontal extension
    ),
    white: coords(9, [1, 7], [7, 1], [1, 3], [7, 5]),
    highlight: [
      { id: coordsOne(9, 6, 2), label: '5' },
      { id: coordsOne(9, 5, 4), label: '5' },
    ],
    label: '9 × 9 · double',
    title: 'Double-direction threat',
    description:
      'Black threatens five-in-a-row in <em>two</em> directions simultaneously — ' +
      'along the diagonal and the horizontal. Each <strong>5</strong> label marks a winning square. ' +
      'White can only cover one; the other wins. ' +
      'Building converging threats like this is the hallmark of strong Gomoku play.',
  },

  {
    id: 'tria-forbidden',
    width: 9,
    black: coords(9, [3, 4], [4, 4], [3, 5], [4, 5]),
    white: coords(9, [6, 3], [2, 6], [6, 6]),
    highlight: [
      { id: coordsOne(9, 5, 3), label: '✕' },
      { id: coordsOne(9, 2, 4), label: '✕' },
    ],
    label: '9 × 9 · tria rule',
    title: 'Tria — double open-three forbidden',
    description:
      'The <strong>✕</strong> squares would each complete an <em>open three</em> for Black. ' +
      'Playing either one simultaneously creates a second open three — a "double tria" — ' +
      'which is <strong>forbidden for Black</strong> under Pente rules. ' +
      'This restriction prevents Black from forcing an unstoppable open four too easily, ' +
      'and requires thinking several moves ahead to set up legal winning lines.',
  },

  {
    id: 'capture-chain',
    width: 9,
    black: coords(9,
      [4, 4],
      [2, 2], [3, 2],
      [5, 6], [6, 6],
      [2, 6], [3, 5],
    ),
    white: coords(9,
      [1, 2], [4, 2],   // flanking pair 1
      [4, 6], [7, 6],   // flanking pair 2
      [1, 7], [4, 4],   // flanking pair 3 (last)
      [3, 3], [7, 3],
    ),
    highlight: [
      { id: coordsOne(9, 1, 2), label: '4' },
      { id: coordsOne(9, 4, 6), label: '4' },
    ],
    label: '9 × 9 · chain',
    title: 'Capt  ure chain — approaching 10',
    description:
      'White has already made <strong>3 captures</strong> (6 black stones removed) and now ' +
      'threatens two more flanking sequences marked <strong>4</strong>. ' +
      'In Pente, 5 captures equals an instant win, so White is one double-capture away. ' +
      'Black must interrupt the chain or achieve five-in-a-row first — a classic race.',
  },

  {
    id: 'endgame-five',
    width: 13,
    black: coords(13,
      [3, 6], [4, 6], [5, 6], [6, 6], [7, 6],  // five in a row
      [5, 4], [7, 8], [4, 9],
    ),
    white: coords(13,
      [3, 5], [8, 5], [6, 4], [5, 8],
      [9, 7], [2, 7], [8, 8],
    ),
    highlight: [{ id: coordsOne(13, 7, 6), label: '★' }],
    label: '13 × 13 · win',
    title: 'Endgame — five-in-a-row sealed',
    description:
      '<strong>★</strong> marks Black\'s final stone completing five in a row. ' +
      'Despite White\'s capture threats on either flank, Black found a clean ' +
      'unblocked line through the centre. The position shows how a decisive ' +
      'horizontal five can appear even in a cluttered endgame — ' +
      'always scan every open line before committing to a capture race.',
  },
]