<template>
    <div class="edit-container">
        <!-- 顶部导航 -->
        <div class="edit-header">
            <div class="header-left">
                <button class="btn-icon" @click="$router.back()">
                    <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                        <path d="M19 12H5m7 7l-7-7 7-7" />
                    </svg>
                </button>
                <h1 class="page-title">{{ isEdit ? '编辑题目' : '新建题目' }} <span v-if="isEdit" class="id-badge">#{{ problemId
                        }}</span></h1>
            </div>
            <div class="header-actions">
                <button type="button" class="btn btn-secondary" @click="$router.back()">取消</button>
                <button type="button" class="btn btn-primary" @click="submitForm" :disabled="submitting">
                    <span v-if="submitting" class="spinner-sm"></span>
                    {{ submitting ? '保存中...' : '保存更改' }}
                </button>
            </div>
        </div>

        <div class="main-grid">
            <!-- 左侧：核心信息 -->
            <div class="left-col">
                <div class="card form-section">
                    <div class="form-group">
                        <label class="form-label">题目名称</label>
                        <input v-model="form.title" class="input-lg" placeholder="请输入题目名称，如：A+B Problem" required />
                    </div>
                </div>

                <div class="card form-section">
                    <div class="editor-toolbar">
                        <span class="label">题目描述 (支持 Markdown & LaTeX)</span>
                        <div class="view-toggle">
                            <button type="button" :class="{ active: !previewMode }"
                                @click="previewMode = false">编辑</button>
                            <button type="button" :class="{ active: previewMode }"
                                @click="previewMode = true">预览</button>
                        </div>
                    </div>

                    <div class="editor-container">
                        <!-- 编辑模式 -->
                        <textarea v-show="!previewMode" v-model="form.description" class="markdown-editor"
                            placeholder="支持 **加粗**，$E=mc^2$ 公式，以及 ```代码块```"></textarea>

                        <!-- 预览模式 -->
                        <div v-show="previewMode" class="markdown-preview markdown-body"
                            v-html="renderedDescription || '<p class=text-muted>暂无内容</p>'"></div>
                    </div>
                    <div class="editor-footer">
                        <small>提示：使用标准的 Markdown 语法。数学公式使用 $...$ 包裹。</small>
                    </div>
                </div>

                <div class="card form-section">
                    <div class="io-grid">
                        <div class="form-group">
                            <label class="form-label">样例输入</label>
                            <textarea v-model="form.sample_input" class="input-code" rows="5"
                                placeholder="例如：1 2"></textarea>
                        </div>
                        <div class="form-group">
                            <label class="form-label">样例输出</label>
                            <textarea v-model="form.sample_output" class="input-code" rows="5"
                                placeholder="例如：3"></textarea>
                        </div>
                    </div>
                </div>
            </div>

            <!-- 右侧：配置与数据 -->
            <div class="right-col">
                <!-- 限制配置 -->
                <div class="card config-card">
                    <h3 class="card-title">评测限制</h3>
                    <div class="form-group">
                        <label class="form-label">时间限制 (ms)</label>
                        <div class="input-suffix-wrapper">
                            <input
                                type="number"
                                v-model.number="form.time_limit"
                                class="input-control"
                                min="0"
                                max="100000"
                                step="1"
                            />
                            <span class="suffix">ms</span>
                        </div>
                    </div>
                    <div class="form-group">
                        <label class="form-label">内存限制 (KB)</label>
                        <div class="input-suffix-wrapper">
                            <input
                                type="number"
                                v-model.number="form.mem_limit"
                                class="input-control"
                                min="0"
                                max="1048576"
                                step="1"
                            />
                            <span class="suffix">KB</span>
                        </div>
                    </div>
                </div>

                <!-- 测试数据上传 -->
                <div class="card upload-card">
                    <h3 class="card-title">测试数据</h3>
                    <p class="help-text">请上传包含 .in 和 .out 文件的 ZIP 压缩包。不需要包含文件夹结构。</p>

                    <div class="upload-area" :class="{ 'has-file': fileInput?.files?.length }">
                        <input type="file" ref="fileInput" accept=".zip" id="zip-upload" @change="handleFileSelect"
                            hidden />
                        <label for="zip-upload" class="upload-label">
                            <div class="icon-cloud">☁️</div>
                            <span v-if="selectedFileName">{{ selectedFileName }}</span>
                            <span v-else>点击选择 ZIP 文件</span>
                        </label>
                    </div>

                    <div v-if="isEdit" class="upload-actions">
                        <button type="button" class="btn btn-outline btn-block" @click="uploadTestcases"
                            :disabled="uploading || !selectedFileName">
                            <span v-if="uploading" class="spinner-sm dark"></span>
                            {{ uploading ? '正在上传解压...' : '上传并更新' }}
                        </button>
                    </div>
                    <div v-else class="upload-blocked">
                        ⚠️ 请先保存题目生成 ID 后再上传数据
                    </div>

                    <div v-if="uploadMsg" class="upload-feedback" :class="uploadSuccess ? 'success' : 'error'">
                        {{ uploadMsg }}
                    </div>

                    <div class="form-group mt-4">
                        <label class="form-label">数据路径 (自动生成)</label>
                        <input v-model="form.tc_path" class="input-control readonly" readonly placeholder="上传后自动填充" />
                    </div>
                </div>
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
import { ref, computed, reactive, onMounted } from 'vue'
import { useRoute, useRouter } from 'vue-router'
// 假设你已经有这个工具函数，如果没有，请参考之前的 ProblemDetail.vue 引入 markdown-it
import MarkdownIt from 'markdown-it'
import mk from '@iktakahiro/markdown-it-katex'
import hljs from 'highlight.js'
import 'highlight.js/styles/github.css'
import 'katex/dist/katex.min.css'

// 简单的 Markdown 初始化
const md = new MarkdownIt({
    html: true,
    linkify: true,
    typographer: true,
    highlight: (str, lang) => {
        if (lang && hljs.getLanguage(lang)) {
            try { return hljs.highlight(str, { language: lang, ignoreIllegals: true }).value } catch (__) { }
        }
        return ''
    }
})
md.use(mk)

const route = useRoute()
const router = useRouter()
const previewMode = ref(false)
const problemId = route.params.id
const isEdit = computed(() => !!problemId)
const submitting = ref(false)

const form = reactive({
    title: '',
    time_limit: 1000,
    mem_limit: 65536,
    description: '',
    sample_input: '',
    sample_output: '',
    tc_path: ''
})

const renderedDescription = computed(() => {
    if (!form.description) return ''
    return md.render(form.description)
})

async function fetchDetails() {
    if (!isEdit.value) return
    try {
        const res = await fetch(`/api/problems/${problemId}`)
        if (res.ok) {
            Object.assign(form, await res.json())
        } else {
            alert("题目不存在或加载失败")
        }
    } catch (e) {
        alert("网络错误")
    }
}

async function submitForm() {
    const token = localStorage.getItem('token')
    if (!token) return alert('请先登录')

    const TIME_MIN = 0
    const TIME_MAX = 100000 // 100s
    const MEM_MIN = 0
    const MEM_MAX = 1048576 // 1GB in KB

    const toInt = (value: unknown) => {
        const n = typeof value === 'number' ? value : Number(value)
        if (!Number.isFinite(n)) return null
        return Math.trunc(n)
    }

    const rawTime = form.time_limit
    const rawMem = form.mem_limit
    const t = toInt(form.time_limit)
    const m = toInt(form.mem_limit)

    const problems: string[] = []
    if (t === null) problems.push(`时间限制不是合法数字：${String(rawTime)}`)
    else if (t < TIME_MIN || t > TIME_MAX) problems.push(`时间限制超出范围：${t}（允许 ${TIME_MIN}~${TIME_MAX} ms）`)

    if (m === null) problems.push(`内存限制不是合法数字：${String(rawMem)}`)
    else if (m < MEM_MIN || m > MEM_MAX) problems.push(`内存限制超出范围：${m}（允许 ${MEM_MIN}~${MEM_MAX} KB）`)

    if (problems.length) {
        alert('评测限制填写不合法，已取消提交：\n' + problems.map((x) => `- ${x}`).join('\n'))
        return
    }

    // Normalize to integers before submit
    form.time_limit = t as number
    form.mem_limit = m as number

    submitting.value = true
    const url = isEdit.value ? `/api/admin/problems/${problemId}` : `/api/admin/problems`
    const method = isEdit.value ? 'PUT' : 'POST'

    try {
        const res = await fetch(url, {
            method,
            headers: { 'Content-Type': 'application/json', 'Authorization': `Bearer ${token}` },
            body: JSON.stringify(form)
        })

        if (!res.ok) throw new Error((await res.json()).message || '保存失败')

        const data = await res.json()
        const id = isEdit.value ? problemId : data.new_id

        // 成功提示可以使用 Toast，这里先用 alert
        // alert('保存成功')
        router.push({ name: 'problem-detail', params: { id } })
    } catch (e: any) {
        alert('保存出错: ' + e.message)
    } finally {
        submitting.value = false
    }
}

// 文件上传逻辑
const fileInput = ref<HTMLInputElement | null>(null)
const selectedFileName = ref('')
const uploading = ref(false)
const uploadMsg = ref('')
const uploadSuccess = ref(false)

function handleFileSelect(event: Event) {
    const target = event.target as HTMLInputElement
    if (target.files && target.files.length > 0) {
        selectedFileName.value = target.files[0].name
        uploadMsg.value = '' // 清除旧消息
    } else {
        selectedFileName.value = ''
    }
}

async function uploadTestcases() {
    if (!fileInput.value?.files?.length) return

    uploading.value = true
    uploadMsg.value = ''

    const formData = new FormData()
    formData.append('file', fileInput.value.files[0])

    try {
        const token = localStorage.getItem('token')
        const res = await fetch(`/api/admin/problems/${problemId}/testcases`, {
            method: 'POST',
            headers: { 'Authorization': `Bearer ${token}` },
            body: formData
        })

        const data = await res.json()
        if (res.ok && data.code === 200) {
            uploadSuccess.value = true
            uploadMsg.value = '上传成功！'
            if (data.tc_path) form.tc_path = data.tc_path
        } else {
            throw new Error(data.message || '上传失败')
        }
    } catch (e: any) {
        uploadSuccess.value = false
        uploadMsg.value = e.message
    } finally {
        uploading.value = false
    }
}

onMounted(fetchDetails)
</script>

<style scoped>
/* 样式变量 */
:root {
    --primary: #4f46e5;
    --primary-hover: #4338ca;
    --bg-page: #f8fafc;
    --border: #e2e8f0;
    --text-main: #1e293b;
    --text-muted: #64748b;
    --radius: 8px;
}

.edit-container {
    max-width: 1200px;
    margin: 0 auto;
    padding: 1.5rem;
    color: var(--text-main);
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
}

/* 顶部 Header */
.edit-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 2rem;
}

.header-left {
    display: flex;
    align-items: center;
    gap: 1rem;
}

.btn-icon {
    background: none;
    border: none;
    cursor: pointer;
    color: var(--text-muted);
    padding: 4px;
    border-radius: 4px;
    display: flex;
}

.btn-icon:hover {
    background: #e2e8f0;
    color: var(--text-main);
}

.page-title {
    font-size: 1.5rem;
    font-weight: 700;
    margin: 0;
    display: flex;
    align-items: center;
    gap: 10px;
}

.id-badge {
    font-size: 0.9rem;
    background: #e2e8f0;
    padding: 2px 8px;
    border-radius: 4px;
    color: var(--text-muted);
    font-family: monospace;
    font-weight: normal;
}

.header-actions {
    display: flex;
    gap: 10px;
}

/* 布局网格 */
.main-grid {
    display: grid;
    grid-template-columns: 1fr 320px;
    gap: 1.5rem;
}

/* 卡片样式 */
.card {
    background: white;
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 1.5rem;
    margin-bottom: 1.5rem;
    box-shadow: 0 1px 2px 0 rgba(0, 0, 0, 0.05);
}

.form-group {
    margin-bottom: 1.2rem;
}

.form-label {
    display: block;
    font-weight: 600;
    font-size: 0.9rem;
    margin-bottom: 0.5rem;
    color: #334155;
}

/* 输入框通用 */
input,
textarea {
    width: 100%;
    padding: 0.6rem 0.8rem;
    border: 1px solid var(--border);
    border-radius: 6px;
    font-size: 0.95rem;
    transition: border-color 0.2s;
    box-sizing: border-box;
}

input:focus,
textarea:focus {
    outline: none;
    border-color: var(--primary);
    box-shadow: 0 0 0 3px rgba(79, 70, 229, 0.1);
}

.input-lg {
    font-size: 1.2rem;
    padding: 0.8rem;
    font-weight: 600;
}

.input-code {
    font-family: 'Monaco', monospace;
    font-size: 0.9rem;
    background: #f8fafc;
}

.input-control {
    background: white;
}

.input-control.readonly {
    background: #f1f5f9;
    color: var(--text-muted);
    cursor: not-allowed;
}

/* 带后缀的输入框 */
.input-suffix-wrapper {
    position: relative;
    display: flex;
    align-items: center;
}

.input-suffix-wrapper input {
    padding-right: 3rem;
}

.suffix {
    position: absolute;
    right: 10px;
    color: var(--text-muted);
    font-size: 0.85rem;
    pointer-events: none;
}

/* 编辑器区域 */
.editor-toolbar {
    display: flex;
    justify-content: space-between;
    align-items: center;
    border-bottom: 1px solid var(--border);
    padding-bottom: 10px;
    margin-bottom: 10px;
}

.label {
    font-weight: 600;
    font-size: 0.9rem;
}

.view-toggle {
    background: #f1f5f9;
    padding: 2px;
    border-radius: 6px;
    display: flex;
}

.view-toggle button {
    border: none;
    background: none;
    padding: 4px 12px;
    font-size: 0.85rem;
    cursor: pointer;
    border-radius: 4px;
    color: var(--text-muted);
}

.view-toggle button.active {
    background: white;
    color: var(--primary);
    font-weight: 600;
    box-shadow: 0 1px 2px rgba(0, 0, 0, 0.05);
}

.editor-container {
    min-height: 400px;
    position: relative;
}

.markdown-editor {
    height: 400px;
    border: none;
    resize: vertical;
    padding: 0;
    font-family: 'Monaco', monospace;
    line-height: 1.6;
}

.markdown-editor:focus {
    border: none;
    box-shadow: none;
}

.markdown-preview {
    height: 400px;
    overflow-y: auto;
    padding: 0 10px;
    border-left: 1px solid transparent;
}

.editor-footer {
    border-top: 1px solid var(--border);
    padding-top: 10px;
    font-size: 0.8rem;
    color: var(--text-muted);
}

/* IO Grid */
.io-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 1.5rem;
}

/* 右侧栏特定 */
.config-card .card-title,
.upload-card .card-title {
    margin: 0 0 1rem 0;
    font-size: 1rem;
    color: #0f172a;
    border-left: 3px solid var(--primary);
    padding-left: 8px;
}

/* 上传区域 */
.upload-area {
    border: 2px dashed var(--border);
    border-radius: 8px;
    padding: 1.5rem;
    text-align: center;
    transition: all 0.2s;
    cursor: pointer;
    background: #f8fafc;
}

.upload-area:hover {
    border-color: var(--primary);
    background: #eef2ff;
}

.upload-area.has-file {
    border-color: #10b981;
    background: #ecfdf5;
}

.upload-label {
    cursor: pointer;
    display: block;
}

.icon-cloud {
    font-size: 2rem;
    margin-bottom: 0.5rem;
}

.upload-actions {
    margin-top: 1rem;
}

.upload-blocked {
    margin-top: 1rem;
    font-size: 0.85rem;
    color: #f59e0b;
    background: #fffbeb;
    padding: 8px;
    border-radius: 4px;
    text-align: center;
}

.upload-feedback {
    margin-top: 10px;
    font-size: 0.85rem;
    text-align: center;
    padding: 6px;
    border-radius: 4px;
}

.upload-feedback.success {
    background: #d1fae5;
    color: #047857;
}

.upload-feedback.error {
    background: #fee2e2;
    color: #b91c1c;
}

.help-text {
    font-size: 0.8rem;
    color: var(--text-muted);
    margin-bottom: 1rem;
}

.mt-4 {
    margin-top: 1rem;
}

/* 按钮 */
.btn {
    padding: 8px 16px;
    border-radius: 6px;
    border: none;
    font-weight: 500;
    cursor: pointer;
    transition: all 0.2s;
    font-size: 0.9rem;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    gap: 6px;
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

.btn-secondary {
    background: white;
    border: 1px solid var(--border);
    color: var(--text-main);
}

.btn-secondary:hover {
    background: #f1f5f9;
}

.btn-outline {
    background: white;
    border: 1px solid var(--primary);
    color: var(--primary);
}

.btn-outline:hover:not(:disabled) {
    background: #eef2ff;
}

.btn-outline:disabled {
    border-color: var(--border);
    color: var(--text-muted);
    cursor: not-allowed;
}

.btn-block {
    width: 100%;
}

/* Spinner */
.spinner-sm {
    width: 14px;
    height: 14px;
    border: 2px solid rgba(255, 255, 255, 0.3);
    border-top-color: white;
    border-radius: 50%;
    animation: spin 1s linear infinite;
}

.spinner-sm.dark {
    border-color: rgba(0, 0, 0, 0.1);
    border-top-color: var(--primary);
}

@keyframes spin {
    to {
        transform: rotate(360deg);
    }
}

/* Markdown 样式覆盖 (简单版) */
:deep(.markdown-preview p) {
    margin-bottom: 0.8em;
}

:deep(.markdown-preview pre) {
    background: #f1f5f9;
    padding: 10px;
    border-radius: 6px;
    overflow-x: auto;
}

:deep(.markdown-preview code) {
    background: rgba(0, 0, 0, 0.05);
    padding: 2px 4px;
    border-radius: 4px;
    font-family: monospace;
}

@media (max-width: 900px) {
    .main-grid {
        grid-template-columns: 1fr;
    }

    .io-grid {
        grid-template-columns: 1fr;
    }
}
</style>