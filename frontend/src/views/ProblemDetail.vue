<template>
    <div class="problem-container">
        <!-- 顶部导航与操作栏 -->
        <div class="nav-header">
            <RouterLink :to="{ name: 'problem-list' }" class="back-link">
                <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                    <path d="M19 12H5m7 7l-7-7 7-7" />
                </svg>
                返回列表
            </RouterLink>
            <div class="problem-meta-id" v-if="problem">#{{ problem.id }}</div>
        </div>

        <!-- 加载与错误状态 -->
        <div v-if="loading" class="loading-state">
            <div class="spinner"></div>
            <p>题目数据加载中...</p>
        </div>
        <div v-else-if="error" class="error-state">
            <div class="error-icon">⚠️</div>
            <h3>加载失败</h3>
            <p>{{ error }}</p>
            <button @click="load" class="btn btn-secondary">重试</button>
        </div>

        <!-- 主内容区：双栏布局 -->
        <div v-else-if="problem" class="main-layout">

            <!-- 左侧：题目描述 -->
            <div class="left-column card">
                <div class="problem-header">
                    <h1 class="problem-title">{{ problem.title }}</h1>
                    <div class="problem-tags">
                        <span class="tag">时间限制: {{ problem.time_limit }}ms</span>
                        <span class="tag">内存限制: {{ problem.mem_limit }}KB</span>
                    </div>
                </div>

                <div class="problem-body">
                    <h3>题目描述</h3>
                    <div class="markdown-body" v-html="renderedDescription"></div>

                    <h3>输入格式</h3>
                    <div class="io-sample">
                        <pre>{{ problem.sample_input }}</pre>
                        <div class="copy-btn" @click="copy(problem.sample_input)">复制</div>
                    </div>

                    <h3>输出格式</h3>
                    <div class="io-sample">
                        <pre>{{ problem.sample_output }}</pre>
                    </div>
                </div>
            </div>

            <!-- 右侧：提交面板与信息 -->
            <div class="right-column">

                <!-- 统计卡片 -->
                <div class="card stat-card">
                    <div class="stat-item">
                        <span class="label">通过率</span>
                        <span class="value success">{{ passRate }}%</span>
                    </div>
                    <div class="divider"></div>
                    <div class="stat-item">
                        <span class="label">提交数</span>
                        <span class="value">{{ problem.sub_count }}</span>
                    </div>
                    <div class="divider"></div>
                    <div class="stat-item">
                        <span class="label">通过数</span>
                        <span class="value">{{ problem.ac_count }}</span>
                    </div>
                </div>

                <!-- 提交代码卡片 -->
                <div class="card submit-card">
                    <div class="card-header">
                        <h3>提交代码</h3>
                        <span class="status-dot" :class="{ active: authState.token }"
                            :title="authState.token ? '已登录' : '未登录'"></span>
                    </div>

                    <button
                        @click="gotoSubmitPage"
                        class="btn btn-primary btn-block submit-entry"
                        :title="authState.token ? '打开提交页面' : '登录后提交'"
                    >
                        {{ authState.token ? '去提交' : '登录后提交' }}
                    </button>
                </div>

                <!-- 我的提交记录卡片 -->
                <div class="card history-card">
                    <div class="card-header">
                        <h3>我的提交</h3>
                        <button class="btn-text" @click="loadSubmissions" :disabled="subLoading">刷新</button>
                    </div>

                    <div v-if="!authState.token" class="empty-state">
                        请登录查看记录
                    </div>
                    <div v-else-if="subLoading" class="loading-sm">
                        加载中...
                    </div>
                    <div v-else-if="!submissions.length" class="empty-state">
                        暂无提交记录
                    </div>
                    <ul v-else class="submission-list">
                        <li v-for="s in submissions" :key="s.id" class="sub-item">
                            <div class="sub-info">
                                <span class="sub-status" :style="{ color: statusColor(s.status) }">
                                    {{ s.status }}
                                </span>
                                <span class="sub-meta">{{ s.time_cost }}ms / {{ s.mem_cost }}KB</span>
                            </div>
                            <div class="sub-right">
                                <span class="sub-lang">{{ s.language }}</span>
                                <RouterLink :to="{ name: 'submission-detail', params: { id: s.id } }"
                                    class="detail-link">
                                    #{{ s.id }} &rsaquo;
                                </RouterLink>
                            </div>
                        </li>
                    </ul>
                </div>

            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
import { onMounted, ref, watch, computed } from 'vue'
import { RouterLink, useRoute, useRouter } from 'vue-router'
import { apiGet } from '../utils/api'
import { authState, refreshUserInfo } from '../utils/auth'

import MarkdownIt from 'markdown-it'
import mk from '@iktakahiro/markdown-it-katex'
import hljs from 'highlight.js'
import 'highlight.js/styles/github.css' // 代码高亮样式
import 'katex/dist/katex.min.css'       // 公式样式

// 初始化 Markdown 解析器
const md: MarkdownIt = new MarkdownIt({
    html: true,
    linkify: true,
    typographer: true,
    highlight: (str: string, lang: string) => {
        if (lang && hljs.getLanguage(lang)) {
            try {
                return '<pre class="hljs"><code>' +
                    hljs.highlight(str, { language: lang, ignoreIllegals: true }).value +
                    '</code></pre>';
            } catch (__) { }
        }
        return '<pre class="hljs"><code>' + md.utils.escapeHtml(str) + '</code></pre>';
    }
})
// 启用 Latex 插件
md.use(mk)

type ProblemDetail = {
    id: number
    title: string
    time_limit: number
    mem_limit: number
    description: string
    sample_input: string
    sample_output: string
    created_time: string
    tc_path: string
    sub_count: number
    ac_count: number
}

const route = useRoute()
const router = useRouter()

const problem = ref<ProblemDetail | null>(null)
const loading = ref(false)
const error = ref('')

// 渲染 Markdown
const renderedDescription = computed(() => {
    if (!problem.value?.description) return ''
    // 简单的预处理：有些题目里的换行可能不规范，可按需处理
    return md.render(problem.value.description)
})

const passRate = computed(() => {
    if (!problem.value || !problem.value.sub_count) return 0
    return Math.round((problem.value.ac_count / Math.max(1, problem.value.sub_count)) * 100)
})

type SubmissionRow = {
    id: number
    problem_id: number
    user_id: number
    language: string
    status: string
    submit_time: string
    time_cost: number
    mem_cost: number
}

const submissions = ref<SubmissionRow[]>([])
const subLoading = ref(false)

function statusColor(status: string) {
    const s = (status || '').toLowerCase()
    if (s.includes('accept')) return '#10b981' // Green
    if (s.includes('wrong') || s.includes('wa')) return '#ef4444' // Red
    if (s.includes('tle') || s.includes('time')) return '#f59e0b' // Orange
    if (s.includes('mle') || s.includes('mem')) return '#f59e0b'
    if (s.includes('compile') || s.includes('ce')) return '#8b5cf6' // Purple
    if (s.includes('run') || s.includes('judg') || s.includes('submit') || s.includes('pending')) return '#3b82f6' // Blue
    return '#64748b'
}

// function formatStatus(status: string) {
//     if (status === 'Accepted') return 'Accepted'
//     if (status === 'Wrong Answer') return 'Wrong'
//     if (status === 'Time Limit Exceeded') return 'Time Limit Exceeded'
//     if (status === 'Memory Limit Exceeded') return 'Memory Limit Exceeded'
//     if (status === 'Compile Error') return 'Compile Error'
//     if (status === 'Runtime Error') return 'Runtime Error'
//     if (status === 'Pending') return 'Pending'
//     if (status === 'Submitted') return 'Submitted'
//     return status
// }

function getId(): number {
    const raw = route.params.id
    const id = typeof raw === 'string' ? Number(raw) : Number(raw?.[0])
    return Number.isFinite(id) ? id : NaN
}

async function gotoSubmitPage() {
    const id = getId()
    if (!Number.isFinite(id)) return

    const target = { name: 'problem-submit' as const, params: { id } }
    const resolved = router.resolve(target)

    if (!authState.token) {
        await router.push({ name: 'login', query: { redirect: resolved.fullPath } })
        return
    }

    await router.push(target)
}

async function load() {
    const id = getId()
    if (!Number.isFinite(id)) {
        error.value = '无效的题目 ID'
        problem.value = null
        return
    }

    loading.value = true
    error.value = ''
    problem.value = null

    try {
        const resp = await fetch(`/api/problems/${id}`)
        if (!resp.ok) throw new Error(`HTTP ${resp.status}`)
        problem.value = await resp.json()
    } catch (e: any) {
        error.value = e?.message ?? String(e)
    } finally {
        loading.value = false
    }
}

async function loadSubmissions() {
    const id = getId()
    if (!Number.isFinite(id)) return
    if (!authState.token) {
        submissions.value = []
        return
    }

    if (!authState.userId) {
        await refreshUserInfo(false)
    }
    if (!authState.userId) return

    subLoading.value = true
    try {
        submissions.value = await apiGet<SubmissionRow[]>(
            `/api/submit/problem/${id}/user/${authState.userId}`,
            { auth: false },
        )
    } catch (e: any) {
        submissions.value = []
    } finally {
        subLoading.value = false
    }
}

function copy(text: string) {
    navigator.clipboard.writeText(text).then(() => {
        // 可添加轻提示
        console.log('已复制')
    })
}

onMounted(() => {
    load()
    loadSubmissions()
})

watch(() => route.params.id, () => {
    load()
    loadSubmissions()
})
</script>

<style scoped>
/* 引入全局变量 */
:global(:root) {
    --primary: #4f46e5;
    --primary-hover: #4338ca;
    --bg-page: #f8fafc;
    --bg-card: #ffffff;
    --text-main: #1e293b;
    --text-muted: #64748b;
    --border: #e2e8f0;
    --radius: 12px;
    --shadow: 0 1px 3px 0 rgb(0 0 0 / 0.1), 0 1px 2px -1px rgb(0 0 0 / 0.1);
}

.problem-container {
    max-width: 1200px;
    margin: 0 auto;
    padding: 2rem 1rem;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    color: var(--text-main);
}

/* 导航头 */
.nav-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 1.5rem;
}

.back-link {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    color: var(--text-muted);
    text-decoration: none;
    font-weight: 500;
    transition: color 0.2s;
}

.back-link:hover {
    color: var(--primary);
}

.problem-meta-id {
    font-family: 'Monaco', monospace;
    background: #e2e8f0;
    padding: 2px 8px;
    border-radius: 4px;
    color: var(--text-muted);
    font-size: 0.9rem;
}

/* 布局：Grid 双栏 */
.main-layout {
    display: grid;
    /* 关键点1：左侧 1fr，右侧固定 320px */
    grid-template-columns: minmax(0, 1fr) 320px;
    /* minmax(0, 1fr) 是防止 grid item 被内容撑大的核心技巧 */
    gap: 1.5rem;
    align-items: start;
}

/* 卡片通用样式 */
.card {
    background: white;
    border-radius: 12px;
    padding: 1.5rem;
    box-shadow: 0 4px 6px -1px rgb(0 0 0 / 0.05);
    border: 1px solid #f1f5f9;
}

/* 左侧栏样式 */
.left-column {
    /* 关键点2：确保容器本身不会溢出 */
    min-width: 0;
    overflow-wrap: break-word;
    /* 强制长文本换行 */
    word-wrap: break-word;
}

.problem-header {
    border-bottom: 1px solid #e2e8f0;
    padding-bottom: 1.5rem;
    margin-bottom: 1.5rem;
}

.problem-title {
    margin: 0 0 1rem 0;
    font-size: 1.8rem;
    color: #0f172a;
    line-height: 1.3;
}

.problem-tags {
    display: flex;
    gap: 10px;
    flex-wrap: wrap;
}

.tag {
    background: #f1f5f9;
    color: var(--text-muted);
    padding: 4px 10px;
    border-radius: 6px;
    font-size: 0.85rem;
}

/* 题目内容样式 (Markdown) */
.problem-body h3 {
    margin: 1.5rem 0 1rem;
    font-size: 1.1rem;
    color: #334155;
    border-left: 4px solid var(--primary);
    padding-left: 10px;
}

/* --- 关键点3：Markdown 内部元素限制 --- */
/* 使用 :deep() 穿透 v-html 生成的内容 */

/* 限制图片宽度 */
:deep(.markdown-body img) {
    max-width: 100%;
    height: auto;
    display: block;
    margin: 1rem auto;
    /* 图片居中 */
    border-radius: 8px;
    box-shadow: 0 2px 4px rgba(0, 0, 0, 0.05);
}

/* 限制代码块和表格滚动 */
:deep(.markdown-body pre),
:deep(.markdown-body table) {
    max-width: 100%;
    overflow-x: auto;
    /* 超出宽度显示滚动条 */
}

/* 基础文字排版 */
:deep(.markdown-body) {
    font-size: 1rem;
    line-height: 1.7;
    color: #334155;
}

:deep(.markdown-body p) {
    margin-bottom: 1rem;
    word-break: break-word;
    /* 防止长单词撑开 */
}

:deep(.markdown-body ul),
:deep(.markdown-body ol) {
    padding-left: 1.5rem;
    margin-bottom: 1rem;
}

:deep(.markdown-body blockquote) {
    border-left: 4px solid #e2e8f0;
    margin: 0 0 1rem 0;
    padding-left: 1rem;
    color: var(--text-muted);
}

:deep(.markdown-body pre) {
    background: #f8fafc;
    /* 稍微深一点的背景 */
    padding: 1rem;
    border-radius: 8px;
    border: 1px solid #e2e8f0;
}

:deep(.markdown-body code) {
    background: rgba(0, 0, 0, 0.05);
    padding: 2px 4px;
    border-radius: 4px;
    font-family: 'Consolas', monospace;
    color: #d946ef;
    font-size: 0.9em;
}

/* 输入输出样例 */
.io-sample {
    position: relative;
    background: #f8fafc;
    border-radius: 8px;
    border: 1px solid #e2e8f0;
    margin-bottom: 1rem;
    overflow: hidden;
    /* 防止内容溢出圆角 */
}

.io-sample pre {
    margin: 0;
    padding: 1rem;
    font-family: 'Consolas', monospace;
    white-space: pre-wrap;
    /* 保留换行 */
    word-break: break-all;
    /* 允许在任意字符间换行，防止长数据撑破 */
    font-size: 0.95rem;
    color: #334155;
    max-height: 300px;
    /* 限制最大高度 */
    overflow-y: auto;
    /* 允许垂直滚动 */
}

.copy-btn {
    position: absolute;
    top: 8px;
    right: 8px;
    font-size: 0.75rem;
    color: var(--text-muted);
    cursor: pointer;
    background: white;
    border: 1px solid #e2e8f0;
    padding: 2px 6px;
    border-radius: 4px;
    opacity: 0;
    transition: opacity 0.2s;
    z-index: 2;
}

.io-sample:hover .copy-btn {
    opacity: 1;
}

/* 右侧栏样式 (保持不变) */
.right-column {
    display: flex;
    flex-direction: column;
    gap: 1.5rem;
    min-width: 0;
    /* 同样防止右侧被内部内容撑大 */
}

/* 统计卡片 */
.stat-card {
    display: flex;
    justify-content: space-around;
    padding: 1rem;
    align-items: center;
}

.stat-item {
    text-align: center;
}

.stat-item .label {
    display: block;
    font-size: 0.75rem;
    color: var(--text-muted);
    margin-bottom: 4px;
}

.stat-item .value {
    font-weight: 700;
    font-size: 1.1rem;
    color: #0f172a;
}

.stat-item .value.success {
    color: #10b981;
}

.divider {
    width: 1px;
    height: 24px;
    background: #e2e8f0;
}

/* 提交卡片 */
.submit-card {
    border-top: 4px solid var(--primary);
}

.submit-entry {
    height: 44px;
    font-size: 0.95rem;
}

.card-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 1rem;
}

.card-header h3 {
    margin: 0;
    font-size: 1rem;
}

.status-dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: #cbd5e1;
}

.status-dot.active {
    background: #10b981;
}

.form-group {
    margin-bottom: 1rem;
}

.form-group label {
    display: block;
    font-size: 0.85rem;
    color: var(--text-muted);
    margin-bottom: 6px;
}

.lang-select {
    width: 100%;
    padding: 8px;
    border-radius: 6px;
    border: 1px solid #cbd5e1;
    background: white;
}

.code-editor-wrapper {
    border: 1px solid #cbd5e1;
    border-radius: 6px;
    overflow: hidden;
    transition: border-color 0.2s;
}

.code-editor-wrapper:focus-within {
    border-color: var(--primary);
}

.code-editor {
    width: 100%;
    height: 200px;
    border: none;
    padding: 10px;
    font-family: 'Monaco', 'Consolas', monospace;
    font-size: 13px;
    resize: vertical;
    outline: none;
    background: #f8fafc;
}

.btn-block {
    width: 100%;
}

/* 提交记录列表 */
.submission-list {
    list-style: none;
    padding: 0;
    margin: 0;
}

.sub-item {
    padding: 10px 0;
    border-bottom: 1px solid #f1f5f9;
    display: flex;
    justify-content: space-between;
    align-items: center;
}

.sub-item:last-child {
    border-bottom: none;
}

.sub-info {
    display: flex;
    flex-direction: column;
    gap: 2px;
}

.sub-status {
    font-weight: 700;
    font-size: 0.9rem;
}

.sub-meta {
    font-size: 0.75rem;
    color: var(--text-muted);
}

.sub-right {
    display: flex;
    align-items: center;
    gap: 8px;
}

.sub-lang {
    font-size: 0.75rem;
    background: #f1f5f9;
    padding: 2px 6px;
    border-radius: 4px;
}

.detail-link {
    text-decoration: none;
    color: var(--text-muted);
    font-size: 0.85rem;
}

.detail-link:hover {
    color: var(--primary);
}

.btn {
    border: none;
    padding: 8px 16px;
    border-radius: 6px;
    cursor: pointer;
    font-weight: 500;
    transition: all 0.2s;
}

.btn-primary {
    background: var(--primary);
    color: white;
}

.btn-primary:hover:not(:disabled) {
    background: var(--primary-hover);
    transform: translateY(-1px);
}

.btn-primary:disabled {
    opacity: 0.7;
    cursor: not-allowed;
}

.btn-text {
    background: none;
    color: var(--primary);
    padding: 0;
    font-size: 0.85rem;
}

.btn-text:hover {
    text-decoration: underline;
}

.empty-state {
    text-align: center;
    color: var(--text-muted);
    font-size: 0.85rem;
    padding: 1rem 0;
}

.loading-state,
.error-state {
    text-align: center;
    padding: 4rem 0;
}

.spinner {
    width: 40px;
    height: 40px;
    border: 4px solid #e2e8f0;
    border-top-color: var(--primary);
    border-radius: 50%;
    animation: spin 1s linear infinite;
    margin: 0 auto 1rem;
}

.spinner-sm {
    display: inline-block;
    width: 12px;
    height: 12px;
    border: 2px solid rgba(255, 255, 255, 0.3);
    border-top-color: white;
    border-radius: 50%;
    animation: spin 1s linear infinite;
    margin-right: 6px;
}

@keyframes spin {
    to {
        transform: rotate(360deg);
    }
}

/* 响应式 */
@media (max-width: 900px) {
    .main-layout {
        grid-template-columns: 1fr;
    }

    .right-column {
        flex-direction: row;
        flex-wrap: wrap;
    }

    .right-column>.card {
        flex: 1;
        min-width: 280px;
    }

    .code-editor {
        height: 150px;
    }
}
</style>