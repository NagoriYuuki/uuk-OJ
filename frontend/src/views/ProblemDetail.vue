<template>
    <div style="padding: 16px">
        <p>
            <RouterLink :to="{ name: 'problem-list' }">← 返回题目列表</RouterLink>
        </p>

        <button @click="load" :disabled="loading">
            {{ loading ? '加载中...' : '刷新' }}
        </button>

        <p v-if="error" style="color: red">{{ error }}</p>

        <div v-if="problem">
            <h1>#{{ problem.id }} {{ problem.title }}</h1>

            <p style="color: #666">
                时间限制：{{ problem.time_limit }}ms｜内存限制：{{ problem.mem_limit }}KB
            </p>
            <p style="color: #666">
                Accepted: {{ problem.ac_count }} / Submissions: {{ problem.sub_count }}
            </p>

            <h3>题目描述</h3>
            <div class="markdown-body" v-html="renderedDescription"></div>

            <h3>样例输入</h3>
            <pre>{{ problem.sample_input }}</pre>

            <h3>样例输出</h3>
            <pre>{{ problem.sample_output }}</pre>

            <h3 style="margin-top: 18px;">提交代码</h3>
            <div class="card" style="margin-bottom: 12px;">
                <div style="display:flex; gap: 10px; align-items:center; flex-wrap:wrap;">
                    <label class="muted">语言：</label>
                    <select v-model="language">
                        <option value="C++">C++</option>
                        <option value="C">C</option>
                        <option value="Python">Python</option>
                    </select>
                    <span class="muted" v-if="!authState.token">（需要先登录，才能查看提交详情）</span>
                    <button @click="submit" :disabled="submitting">{{ submitting ? '提交中…' : '提交' }}</button>
                    <span v-if="submitMsg" :class="submitOk ? 'muted' : 'error'">{{ submitMsg }}</span>
                </div>
                <textarea v-model="code" rows="14" style="width: 100%; margin-top: 10px; font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;"
                    placeholder="#include <bits/stdc++.h>\nusing namespace std;\nint main(){\n  // ...\n}\n"></textarea>
            </div>

            <h3>本题提交记录</h3>
            <div class="card" style="display:flex; gap: 12px; align-items:center; flex-wrap:wrap; margin-bottom: 10px;">
                <button @click="loadSubmissions" :disabled="subLoading">{{ subLoading ? '加载中…' : '刷新提交列表' }}</button>
                <span class="muted">公开列表；提交详情页需要登录鉴权。</span>
            </div>

            <p v-if="subError" class="error">{{ subError }}</p>

            <table v-if="submissions.length">
                <thead>
                    <tr>
                        <th>ID</th>
                        <th>用户</th>
                        <th>语言</th>
                        <th>状态</th>
                        <th>时间</th>
                        <th>内存</th>
                        <th>提交时间</th>
                    </tr>
                </thead>
                <tbody>
                    <tr v-for="s in submissions" :key="s.id">
                        <td>
                            <RouterLink :to="{ name: 'submission-detail', params: { id: s.id } }">#{{ s.id }}</RouterLink>
                        </td>
                        <td>{{ s.user_id }}</td>
                        <td>{{ s.language }}</td>
                        <td :style="{ color: statusColor(s.status), fontWeight: 700 }">{{ s.status }}</td>
                        <td>{{ s.time_cost }} ms</td>
                        <td>{{ s.mem_cost }} KB</td>
                        <td class="muted">{{ s.submit_time || '-' }}</td>
                    </tr>
                </tbody>
            </table>
            <p v-else-if="!subLoading" class="muted">暂无提交</p>
        </div>
    </div>
</template>

<script setup lang="ts">
import { onMounted, ref, watch, computed } from 'vue'
import { RouterLink, useRoute } from 'vue-router'
import { renderMarkdown } from '../utils/markdown'
import { apiGet, apiPost } from '../utils/api'
import { authState, refreshUserInfo } from '../utils/auth'

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

const problem = ref<ProblemDetail | null>(null)
const loading = ref(false)
const error = ref('')

const renderedDescription = computed(() => {
    return problem.value ? renderMarkdown(problem.value.description) : ''
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
const subError = ref('')

const language = ref('C++')
const code = ref('')
const submitting = ref(false)
const submitMsg = ref('')
const submitOk = ref(false)

function statusColor(status: string) {
    const s = (status || '').toLowerCase()
    if (s.includes('accept')) return '#067647'
    if (s.includes('wrong') || s.includes('wa')) return '#b42318'
    if (s.includes('tle') || s.includes('time')) return '#b54708'
    if (s.includes('mle') || s.includes('mem')) return '#b54708'
    if (s.includes('compile') || s.includes('ce')) return '#6d28d9'
    if (s.includes('run') || s.includes('judg') || s.includes('submit')) return '#2563eb'
    return '#334155'
}

function getId(): number {
    const raw = route.params.id
    const id = typeof raw === 'string' ? Number(raw) : Number(raw?.[0])
    return Number.isFinite(id) ? id : NaN
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
    subLoading.value = true
    subError.value = ''
    try {
        submissions.value = await apiGet<SubmissionRow[]>(`/api/submit/problem/${id}/all`, { auth: false })
    } catch (e: any) {
        subError.value = e?.message ?? String(e)
        submissions.value = []
    } finally {
        subLoading.value = false
    }
}

async function submit() {
    submitMsg.value = ''
    submitOk.value = false

    const id = getId()
    if (!Number.isFinite(id)) {
        submitMsg.value = '无效的题目 ID'
        return
    }

    if (!authState.token) {
        submitMsg.value = '请先登录后再提交'
        return
    }

    if (!code.value.trim()) {
        submitMsg.value = '代码不能为空'
        return
    }

    submitting.value = true
    try {
        if (!authState.userId) await refreshUserInfo(false)
        if (!authState.userId) throw new Error('无法获取用户ID，请重新登录')

        const resp = await apiPost<{ code: number; new_id: number; message: string }>(
            '/api/submit/new',
            {
                problem_id: id,
                language: language.value,
                code: code.value,
                user_id: authState.userId,
            },
            { auth: false },
        )
        submitOk.value = true
        submitMsg.value = `提交成功，ID=${resp.new_id}`
        await loadSubmissions()
    } catch (e: any) {
        submitOk.value = false
        submitMsg.value = e?.message ?? String(e)
    } finally {
        submitting.value = false
    }
}

onMounted(load)
onMounted(loadSubmissions)
watch(
    () => route.params.id,
    () => {
        load()
        loadSubmissions()
    },
)
</script>

<style>
.markdown-body {
    line-height: 1.6;
}
.markdown-body h1, .markdown-body h2, .markdown-body h3 {
    margin-top: 24px;
    margin-bottom: 16px;
    border-bottom: 1px solid #eaecef;
    padding-bottom: .3em;
}
.markdown-body p {
    margin-bottom: 16px;
}
.markdown-body code {
    background-color: rgba(175, 184, 193, 0.2);
    border-radius: 6px;
    padding: .2em .4em;
    font-family: monospace;
}
.markdown-body pre {
    background-color: #f6f8fa;
    border-radius: 6px;
    padding: 16px;
    overflow: auto;
}
.markdown-body pre code {
    background-color: transparent;
    padding: 0;
}
.markdown-body blockquote {
    color: #656d76;
    border-left: .25em solid #d0d7de;
    padding: 0 1em;
    margin: 0;
}
</style>
