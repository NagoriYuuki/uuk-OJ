<template>
    <div class="submit-container">
        <div class="nav-header">
            <RouterLink :to="{ name: 'problem-detail', params: { id } }" class="back-link">
                <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                    <path d="M19 12H5m7 7l-7-7 7-7" />
                </svg>
                返回题目
            </RouterLink>
            <div class="problem-meta-id">#{{ id }}</div>
        </div>

        <div v-if="loading" class="loading-state">
            <div class="spinner"></div>
            <p>页面加载中...</p>
        </div>

        <div v-else class="card">
            <div class="page-header">
                <h1 class="title">提交代码</h1>
                <div class="sub-title" v-if="problemTitle">{{ problemTitle }}</div>
            </div>

            <div class="form">
                <div class="form-group">
                    <label>语言</label>
                    <select v-model="language" class="lang-select">
                        <option value="C++">C++</option>
                        <!-- <option value="C">C</option>
                        <option value="Java">Java</option>
                        <option value="Python">Python</option> -->
                    </select>
                </div>

                <div class="form-group">
                    <label>代码</label>
                    <div class="code-editor-wrapper">
                        <textarea
                            v-model="code"
                            class="code-editor"
                            placeholder="在这里粘贴/编写代码..."
                            spellcheck="false"
                        ></textarea>
                    </div>
                </div>

                <div class="actions">
                    <button class="btn btn-primary" :disabled="submitting || !code.trim()" @click="submit">
                        <span v-if="submitting" class="spinner-sm"></span>
                        {{ submitting ? '提交中...' : '提交' }}
                    </button>
                    <button class="btn btn-secondary" :disabled="submitting" @click="clearDraft">清空</button>
                </div>

                <div v-if="msg" class="msg" :class="{ ok, err: !ok }">{{ msg }}</div>
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
import { computed, onMounted, ref, watch } from 'vue'
import { RouterLink, useRoute, useRouter } from 'vue-router'
import { apiPost } from '../utils/api'
import { authState, refreshUserInfo } from '../utils/auth'

const route = useRoute()
const router = useRouter()

const id = computed(() => {
    const raw = route.params.id
    const v = typeof raw === 'string' ? Number(raw) : Number(raw?.[0])
    return Number.isFinite(v) ? v : NaN
})

const loading = ref(true)
const problemTitle = ref('')

const language = ref('C++')
const code = ref('')

const submitting = ref(false)
const msg = ref('')
const ok = ref(false)

function draftKey() {
    return `draft:submit:${id.value}:${language.value}`
}

function loadDraft() {
    if (!Number.isFinite(id.value)) return
    const saved = localStorage.getItem(draftKey())
    if (saved !== null) code.value = saved
}

function saveDraft() {
    if (!Number.isFinite(id.value)) return
    localStorage.setItem(draftKey(), code.value)
}

function clearDraft() {
    code.value = ''
    if (!Number.isFinite(id.value)) return
    localStorage.removeItem(draftKey())
}

async function loadProblemTitle() {
    if (!Number.isFinite(id.value)) {
        loading.value = false
        return
    }

    try {
        const resp = await fetch(`/api/problems/${id.value}`)
        if (!resp.ok) throw new Error(`HTTP ${resp.status}`)
        const data = await resp.json()
        problemTitle.value = data?.title || ''
    } catch {
        problemTitle.value = ''
    } finally {
        loading.value = false
    }
}

async function submit() {
    msg.value = ''
    ok.value = false

    if (!Number.isFinite(id.value)) {
        msg.value = '无效的题目 ID'
        return
    }

    if (!authState.token) {
        const target = { name: 'problem-submit' as const, params: { id: id.value } }
        const resolved = router.resolve(target)
        await router.push({ name: 'login', query: { redirect: resolved.fullPath } })
        return
    }

    if (!code.value.trim()) {
        msg.value = '代码不能为空'
        return
    }

    submitting.value = true
    try {
        if (!authState.userId) await refreshUserInfo(false)
        if (!authState.userId) throw new Error('用户信息获取失败')

        const resp = await apiPost<{ code: number; new_id: number; message: string }>(
            '/api/submit/new',
            {
                problem_id: id.value,
                language: language.value,
                code: code.value,
                user_id: authState.userId,
            },
        )

        ok.value = true
        msg.value = resp?.message || '提交成功'
        saveDraft()

        if (resp?.new_id) {
            await router.push({ name: 'submission-detail', params: { id: resp.new_id } })
        }
    } catch (e: any) {
        ok.value = false
        msg.value = e?.message ?? String(e)
    } finally {
        submitting.value = false
    }
}

onMounted(async () => {
    await loadProblemTitle()
    loadDraft()
})

watch([id, language], () => {
    msg.value = ''
    ok.value = false
    loadDraft()
})

watch(code, () => {
    saveDraft()
})
</script>

<style scoped>
.submit-container {
    max-width: 900px;
    margin: 0 auto;
    padding: 2rem 1rem;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    color: var(--text-main);
}

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

.card {
    background: white;
    border-radius: 12px;
    padding: 1.5rem;
    box-shadow: 0 4px 6px -1px rgb(0 0 0 / 0.05);
    border: 1px solid #f1f5f9;
}

.page-header {
    margin-bottom: 1rem;
}

.title {
    margin: 0;
    font-size: 1.4rem;
    color: #0f172a;
}

.sub-title {
    margin-top: 6px;
    color: var(--text-muted);
    font-size: 0.95rem;
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
    padding: 10px;
    border-radius: 8px;
    border: 1px solid #cbd5e1;
    background: white;
}

.code-editor-wrapper {
    border: 1px solid #cbd5e1;
    border-radius: 8px;
    overflow: hidden;
    transition: border-color 0.2s;
}

.code-editor-wrapper:focus-within {
    border-color: var(--primary);
}

.code-editor {
    width: 100%;
    height: 420px;
    border: none;
    padding: 12px;
    font-family: 'Monaco', 'Consolas', monospace;
    font-size: 13px;
    resize: vertical;
    outline: none;
    background: #f8fafc;
}

.actions {
    display: flex;
    gap: 10px;
}

.btn {
    border: none;
    padding: 10px 16px;
    border-radius: 8px;
    cursor: pointer;
    font-weight: 600;
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

.btn-secondary {
    background: #eef2ff;
    color: #3730a3;
}

.btn-secondary:hover:not(:disabled) {
    background: #e0e7ff;
}

.btn:disabled {
    opacity: 0.7;
    cursor: not-allowed;
}

.msg {
    margin-top: 12px;
    padding: 10px 12px;
    border-radius: 8px;
    font-size: 0.9rem;
    border: 1px solid transparent;
}

.msg.ok {
    color: #065f46;
    background: #ecfdf5;
    border-color: #a7f3d0;
}

.msg.err {
    color: #991b1b;
    background: #fef2f2;
    border-color: #fecaca;
}

.loading-state {
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

@media (max-width: 900px) {
    .code-editor {
        height: 320px;
    }

    .actions {
        flex-direction: column;
    }

    .actions .btn {
        width: 100%;
    }
}
</style>
