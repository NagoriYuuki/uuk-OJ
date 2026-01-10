<template>
  <div>
    <p class="muted" style="margin: 6px 0 10px;">
      <RouterLink to="/submissions">← 返回提交列表</RouterLink>
    </p>

    <div style="display:flex; align-items:center; justify-content:space-between; gap: 12px; flex-wrap:wrap;">
      <h1 style="margin: 6px 0 10px;">提交 #{{ id }}</h1>
      <div style="display:flex; gap: 10px; align-items:center;">
        <label style="display:flex; gap:6px; align-items:center;" class="muted">
          <input type="checkbox" v-model="autoRefresh" />
          自动刷新
        </label>
        <button @click="load" :disabled="loading">{{ loading ? '加载中…' : '刷新' }}</button>
      </div>
    </div>

    <p v-if="error" class="error">{{ error }}</p>

    <div v-if="sub" class="card" style="display:grid; grid-template-columns: 1fr 1fr; gap: 10px;">
      <div><b>题目</b>：<RouterLink :to="{ name: 'problem-detail', params: { id: sub.problem_id } }">#{{ sub.problem_id }}</RouterLink></div>
      <div><b>用户</b>：{{ sub.user_id }}</div>
      <div><b>语言</b>：{{ sub.language }}</div>
      <div><b>状态</b>：<span :style="{ color: statusColor(sub.status), fontWeight: 800 }">{{ sub.status }}</span></div>
      <div><b>时间</b>：{{ sub.time_cost }} ms</div>
      <div><b>内存</b>：{{ sub.mem_cost }} KB</div>
      <div style="grid-column: 1 / -1;"><b>提交时间</b>：<span class="muted">{{ sub.submit_time || '-' }}</span></div>
    </div>

    <div v-if="sub" class="card" style="margin-top: 12px;">
      <h3 style="margin: 6px 0 10px;">判题详情</h3>
      <pre style="white-space: pre-wrap; margin: 0;">{{ sub.detail || '(无)' }}</pre>
    </div>

    <div v-if="sub" class="card" style="margin-top: 12px;">
      <h3 style="margin: 6px 0 10px;">代码</h3>
      <textarea :value="sub.code" readonly rows="18" style="width: 100%; font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;"></textarea>
    </div>
  </div>
</template>

<script setup lang="ts">
import { onBeforeUnmount, onMounted, ref, watch } from 'vue'
import { RouterLink } from 'vue-router'
import { apiGet } from '../utils/api'

type SubmissionDetail = {
  id: number
  problem_id: number
  user_id: number
  language: string
  code: string
  status: string
  detail: string
  submit_time: string
  time_cost: number
  mem_cost: number
}

const props = defineProps<{ id: number }>()
const id = props.id

const loading = ref(false)
const error = ref('')
const sub = ref<SubmissionDetail | null>(null)
const autoRefresh = ref(true)

let timer: number | null = null

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

function shouldKeepRefreshing(status: string) {
  const s = (status || '').toLowerCase()
  return s.includes('submit') || s.includes('running') || s.includes('judg')
}

async function load() {
  loading.value = true
  error.value = ''
  try {
    sub.value = await apiGet<SubmissionDetail>(`/api/submit/${id}`)
  } catch (e: any) {
    error.value = e?.message ?? String(e)
    sub.value = null
  } finally {
    loading.value = false
  }
}

function startTimer() {
  if (timer != null) return
  timer = window.setInterval(async () => {
    if (!autoRefresh.value) return
    if (!sub.value) {
      await load()
      return
    }
    if (shouldKeepRefreshing(sub.value.status)) await load()
  }, 1200)
}

function stopTimer() {
  if (timer != null) {
    window.clearInterval(timer)
    timer = null
  }
}

onMounted(async () => {
  await load()
  startTimer()
})

onBeforeUnmount(() => {
  stopTimer()
})

watch(autoRefresh, (v) => {
  if (!v) return
  startTimer()
})
</script>
