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
            <pre style="white-space: pre-wrap">{{ problem.description }}</pre>

            <h3>样例输入</h3>
            <pre>{{ problem.sample_input }}</pre>

            <h3>样例输出</h3>
            <pre>{{ problem.sample_output }}</pre>
        </div>
    </div>
</template>

<script setup lang="ts">
import { onMounted, ref, watch } from 'vue'
import { RouterLink, useRoute } from 'vue-router'

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

onMounted(load)
watch(
    () => route.params.id,
    () => load(),
)
</script>
