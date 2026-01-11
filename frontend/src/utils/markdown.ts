import MarkdownIt from 'markdown-it'
import mk from '@iktakahiro/markdown-it-katex';
import DOMPurify from 'isomorphic-dompurify'

// 初始化 markdown-it配置
const md = new MarkdownIt({
    html: true,       // 允许渲染 HTML 标签 (后续会用 DOMPurify 清洗)
    linkify: true,    // 自动识别链接
    typographer: true // 优化排版
})

// 使用 KaTeX 插件，用于渲染 LaTeX 公式
md.use(mk)

/**
 * 渲染 Markdown 文本为 HTML 字符串
 * @param content Markdown 源码
 * @returns 安全的 HTML 字符串
 */
export function renderMarkdown(content: string): string {
    if (!content) return ''
    // 1. 渲染 Markdown -> HTML
    const dirtyHtml = md.render(content)
    // 2. 清洗 HTML，防止 XSS 攻击 (过滤掉危险的 js 脚本等)
    return DOMPurify.sanitize(dirtyHtml)
}
