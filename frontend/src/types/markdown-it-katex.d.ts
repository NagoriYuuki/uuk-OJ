declare module '@iktakahiro/markdown-it-katex' {
  import type MarkdownIt from 'markdown-it'

  const plugin: (md: MarkdownIt, options?: unknown) => void
  export default plugin
}
