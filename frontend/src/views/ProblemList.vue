<template>
    <div style="padding: 16px">
        <h1>题目列表</h1>

        <button @click="load" :disabled="loading">
            {{ loading ? '加载中...' : '刷新' }}
        </button>

        <p v-if="error" style="color: red">{{ error }}</p>

        <ul v-if="problems.length">
            <li v-for="p in problems" :key="p.id">
                <RouterLink :to="{ name: 'problem-detail', params: { id: p.id } }">
                    <strong>#{{ p.id }} &nbsp; {{ p.title }}</strong>
                </RouterLink>
                &nbsp;

                <span style="color: #666">
                    （{{ p.time_limit }}ms / {{ p.mem_limit }}KB）
                </span>
                <span style="color: #666">
                    （Accepted: {{ p.ac_count }} / Submissions: {{ p.sub_count }}）
                </span>
            </li>
        </ul>

        <p v-else-if="!loading">暂无题目</p>
    </div>
</template>

<script setup lang="ts">
import { onMounted, ref } from 'vue'
import { RouterLink } from 'vue-router'

type Problem = {
    id: number
    title: string
    time_limit: number
    mem_limit: number
    sub_count: number
    ac_count: number
}

const problems = ref<Problem[]>([])
const loading = ref(false)
const error = ref('')

async function load() {
    loading.value = true
    error.value = ''
    try {
        const resp = await fetch('/api/problems')
        if (!resp.ok) throw new Error(`HTTP ${resp.status}`)
        problems.value = await resp.json()
    } catch (e: any) {
        error.value = e?.message ?? String(e)
    } finally {
        loading.value = false
    }
}

onMounted(load)
</script>
