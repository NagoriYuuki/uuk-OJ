<template>
  <div>
    <h1 style="margin: 6px 0 14px;">提交记录</h1>

    <div class="card" style="display:flex; gap:12px; align-items:center; flex-wrap:wrap; margin-bottom: 12px;">
      <button @click="load" :disabled="loading">{{ loading ? '加载中…' : '刷新' }}</button>
      <label class="muted">筛选题号：</label>
      <input v-model="filterProblem" placeholder="例如 6" style="width: 120px;" />
      <label class="muted">筛选用户：</label>
      <input v-model="filterUser" placeholder="例如 1" style="width: 120px;" />
      <label class="muted">状态包含：</label>
      <input v-model="filterStatus" placeholder="Accepted / WA" style="width: 160px;" />
      <label style="display:flex; gap:6px; align-items:center;">
        <input type="checkbox" v-model="onlyMine" :disabled="!authState.userId" />
        <span class="muted">只看我的</span>
      </label>
    </div>

    <p v-if="error" class="error">{{ error }}</p>

    <table v-if="filtered.length">
      <thead>
        <tr>
          <th>ID</th>
          <th>题目</th>
          <th>用户</th>
          <th>语言</th>
          <th>状态</th>
          <th>时间</th>
          <th>内存</th>
          <th>提交时间</th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="s in filtered" :key="s.id">
          <td>
            <RouterLink :to="{ name: 'submission-detail', params: { id: s.id } }">#{{ s.id }}</RouterLink>
          </td>
          <td>
            <RouterLink :to="{ name: 'problem-detail', params: { id: s.problem_id } }">#{{ s.problem_id }}</RouterLink>
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
    <p v-else-if="!loading" class="muted">暂无提交记录</p>
    <p v-else class="muted">加载中…</p>

    <p class="muted" style="margin-top: 10px;">
      说明：点提交ID进入详情（需要登录，因为后端详情接口需要 Bearer Token）。
    </p>
  </div>
</template>

<script setup lang="ts">
import { computed, onMounted, ref } from 'vue'
import { RouterLink } from 'vue-router'
import { apiGet } from '../utils/api'
import { authState, refreshUserInfo } from '../utils/auth'

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

const loading = ref(false)
const error = ref('')
const rows = ref<SubmissionRow[]>([])

const filterProblem = ref('')
const filterUser = ref('')
const filterStatus = ref('')
const onlyMine = ref(false)

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

const filtered = computed(() => {
  const p = filterProblem.value.trim()
  const u = filterUser.value.trim()
  const st = filterStatus.value.trim().toLowerCase()

  return rows.value.filter((r) => {
    if (onlyMine.value && authState.userId && r.user_id !== authState.userId) return false
    if (p && String(r.problem_id) !== p) return false
    if (u && String(r.user_id) !== u) return false
    if (st && !(r.status || '').toLowerCase().includes(st)) return false
    return true
  })
})

async function load() {
  loading.value = true
  error.value = ''
  try {
    rows.value = await apiGet<SubmissionRow[]>('/api/submit/all', { auth: false })
  } catch (e: any) {
    error.value = e?.message ?? String(e)
  } finally {
    loading.value = false
  }
}

onMounted(async () => {
  if (authState.token) await refreshUserInfo(false)
  await load()
})
</script>
