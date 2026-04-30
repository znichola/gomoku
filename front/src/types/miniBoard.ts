export type HightLable = {
  id: number,
  label?: string,
  highlight?: boolean,
};

export type Position = {
  id?: string
  width: number
  height?: number
  black: number[]
  white: number[]
  highlight?: HightLable[]
  boardHeight?: string
  label?: string
  title?: string
  description?: string
  accent?: boolean
  history?: number[] // move history as flat cell indices, fed to /replay
}
