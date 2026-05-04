import type { Position } from '@/types/miniBoard'
import { coords, coordsOne, createPosition } from '@/helpers/helpers'

export const positions: Position[] = [
  createPosition({
    width: 0,
    black: [],
    white: [],
    title: 'Goal',
    description:
`
There are two ways to win:
<ul>
  <li>Be the first to align <strong>5 stones</strong> (horizontal, vertical, or diagonal).</li>
  <li>Be the first to capture <strong>10 stones</strong>.</li>
</ul>
`
  }),

  createPosition({
    width: 0,
    black: [],
    white: [],
    title: 'Turn Order',
    description:
`
<ul>
  <li>On your turn, place one stone on any empty intersection.</li>
  <li>Players alternate turns (<span class='black'>Black</span> then <span class='white'>White</span>).</li>
</ul>
`
  }),

  createPosition({
    width: 4,
    white: coords(4, [0, 0]),
    black: coords(4, [1, 1], [2, 2]),
    highlight: [{ id: coordsOne(4, 3, 3), label: 'a' }],
    title: 'Captures',
    cellHeight: '30px',
    description:
`Captures are made by flanking a pair of the opponent's stones. In this scenario, <span class='white'>White</span> plays
on <em>a</em> and captures, removing the two <span class='black'>Black</span> stones from the game.
<br>
One can only capture <em>pairs</em>, not single stones, and not more than 2 stones in a row.
<br>
Captures are incremented in pairs and shown on the HUD (bowl of stones).
`
  }),

  createPosition({
    width: 4,
    height: 1,
    black: coords(5, [0, 0], [3, 0]),
    white: coords(5, [1, 0]),
    highlight: [{ id: coordsOne(5, 2, 0), label: 'a' }],
    title: 'Move into capture',
    cellHeight: '30px',
    description:
`Also note that one cannot move into being captured. In this scenario, <span class='white'>White</span> can 
play in <em>a</em> without losing the pair. 
<br>
However, if later White takes one of the <span class='black'>Black</span> stones, their position becomes vulnerable to capture...
`
  }),

  createPosition({
    width: 6,
    height: 1,
    black: coords(6, [1, 0], [3, 0], [4, 0]),
    white: [],
    highlight: [
      { id: coordsOne(6, 0, 0), label: 'a' },
      { id: coordsOne(6, 2, 0), label: 'b' },
      { id: coordsOne(6, 5, 0), label: 'c' },
    ],
    title: 'Free-three',
    cellHeight: '30px',
    description:
`A <em>free-three</em> is an alignment of three stones that, if not immediately blocked,
allows for an undefendable alignment of four — that is, an open four with both ends unobstructed.
<br>
Here <span class='black'>Black</span> has a free-three (<em>b</em> is a playable gap).
Squares <em>a</em> and <em>c</em> are both open, so extending to four cannot be stopped in one move.
`,
  }),

  createPosition({
    width: 7,
    height: 5,
    black: coords(7, [1, 3], [2, 3], [5, 1], [6, 0]),
    white: [],
    highlight: [
      { id: coordsOne(7, 3, 3), label: 'a' },
      { id: coordsOne(7, 4, 3), label: 'b' }
    ],
    title: 'Double-three',
    cellHeight: '30px',
    description:
`A <em>double-three</em> is a move that simultaneously introduces two free-three alignments.
<br>
It is a <em>forbidden move</em>: you cannot block both threes in a single response.
<br>
Here, <span class='black'>Black</span> cannot play <em>a</em> — it would complete a
horizontal free-three <em>and</em> a vertical free-three at once. 
<br>
If White had a stone on <em>b</em>, then playing in <em>a</em> would be legal, as one of the threes is blocked.
<br>
<em>Exception</em>: introducing a double-three via a capture is still allowed.
`,
  }),

  createPosition({
    width: 8,
    height: 4,
    black: coords(8, [1, 1], [2, 1], [3, 1], [3, 2], [4, 1], [5, 1]),
    white: coords(8, [1, 0]),
    highlight: [
      { id: coordsOne(8, 4, 3), label: 'a' },
      { id: coordsOne(8, 5, 0), label: 'b' }
    ],
    title: 'Endgame capture check',
    cellHeight: '30px',
    description:
`Aligning five stones wins <em>only if</em> the opponent cannot immediately break the line
by capturing a pair from within it.
<br>
Here <span class='black'>Black</span> has five in a row, but <span class='white'>White</span>
can break it by playing on <em>a</em> and capturing the two stones.
<br>
For <span class='black'>Black</span> to win, they can try playing a piece in <em>a</em> to block the potential capture.
And <span class='white'>White</span> can attempt to prolong the game by playing in <em>b</em>, which opens up another potential capture.
`,
  }),
]