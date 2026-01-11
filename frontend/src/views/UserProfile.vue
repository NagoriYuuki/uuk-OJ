<template>
  <div class="profile-page">
    <div class="topbar">
      <RouterLink class="back" to="/">返回首页</RouterLink>
    </div>

    <div class="hero card">
      <div class="hero-left">
        <div class="avatar" aria-hidden="true">{{ initials }}</div>
        <div class="hero-text">
          <div class="title-row">
            <h1 class="page-title">{{ profile ? profile.username : '用户主页' }}</h1>
            <span v-if="profile" class="pill" :class="profile.role === 1 ? 'pill-danger' : 'pill-info'">
              {{ profile.role === 1 ? 'Admin' : 'User' }}
            </span>
          </div>
          <div v-if="profile" class="meta muted">
            提交 {{ profile.sub }} · 通过 {{ profile.ac_count }}
          </div>
          <div v-else class="meta muted">通过后端接口加载用户信息</div>
        </div>
      </div>

      <div class="hero-actions">
        <button class="btn-primary" @click="load" :disabled="loading">{{ loading ? '加载中…' : '刷新' }}</button>
      </div>
    </div>

    <p v-if="error" class="error" style="margin: 12px 0 0;">{{ error }}</p>

    <div v-if="profile" class="stats">
      <div class="stat card">
        <div class="stat-label muted">提交</div>
        <div class="stat-value">{{ profile.sub }}</div>
        <div class="stat-hint muted">总提交次数</div>
      </div>
      <div class="stat card">
        <div class="stat-label muted">AC</div>
        <div class="stat-value">{{ profile.ac_count }}</div>
        <div class="stat-hint muted">通过次数</div>
      </div>
      <div class="stat card">
        <div class="stat-label muted">已通过题目</div>
        <div class="stat-value">{{ profile.ac_problems.length }}</div>
        <div class="stat-hint muted">去题目页查看详情</div>
      </div>
    </div>

    <div v-if="profile" class="section card">
      <div class="section-head">
        <h3 class="section-title">已通过题目</h3>
        <span class="muted">{{ profile.ac_problems.length }} 题</span>
      </div>

      <div v-if="profile.ac_problems.length" class="pill-list">
        <RouterLink
          v-for="pid in profile.ac_problems"
          :key="pid"
          class="pill pill-success"
          :to="{ name: 'problem-detail', params: { id: pid } }"
        >
          #{{ pid }}
        </RouterLink>
      </div>
      <div v-else class="empty muted">暂无 AC</div>
    </div>

    <div v-if="profile" class="section card">
      <div class="section-head" style="gap: 12px;">
        <h3 class="section-title">提交列表</h3>
        <div class="tools">
          <input v-model="query" class="search" placeholder="筛选：题号 / 语言 / 状态" />
        </div>
      </div>

      <div v-if="filteredSubmissions.length" class="table-wrap" style="margin-top: 10px;">
        <table>
          <thead>
            <tr>
              <th>ID</th>
              <th>题目</th>
              <th>语言</th>
              <th>状态</th>
              <th>时间</th>
              <th>内存</th>
              <th>提交时间</th>
            </tr>
          </thead>
          <tbody>
            <tr v-for="s in filteredSubmissions" :key="s.id">
              <td>
                <RouterLink :to="{ name: 'submission-detail', params: { id: s.id } }">#{{ s.id }}</RouterLink>
              </td>
              <td>
                <RouterLink :to="{ name: 'problem-detail', params: { id: s.problem_id } }">
                  #{{ s.problem_id }}
                  <span v-if="problemTitle(s.problem_id)" class="muted"> {{ problemTitle(s.problem_id) }}</span>
                </RouterLink>
              </td>
              <td class="muted">{{ s.language }}</td>
              <td>
                <span class="pill" :class="statusPillClass(s.status)">{{ s.status }}</span>
              </td>
              <td>{{ s.time_cost }} ms</td>
              <td>{{ s.mem_cost }} KB</td>
              <td class="muted">{{ s.submit_time || '-' }}</td>
            </tr>
          </tbody>
        </table>
      </div>
      <div v-else class="empty muted" style="margin-top: 10px;">暂无提交</div>

      <p class="muted" style="margin: 10px 0 0;">
        提示：提交详情页需要登录鉴权（后端 `/api/submit/:id` 要 Bearer Token）。
      </p>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed, onMounted, ref } from 'vue'
import { RouterLink, useRoute } from 'vue-router'
import { apiGet } from '../utils/api'
import { ensureAllProblemTitlesLoaded, getProblemTitle } from '../utils/lookup'

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

type UserProfile = {
  id: number
  username: string
  role: number
  sub: number
  ac_count: number
  submissions: SubmissionRow[]
  ac_problems: number[]
}

const route = useRoute()
const loading = ref(false)
const error = ref('')
const profile = ref<UserProfile | null>(null)

const query = ref('')

function problemTitle(id: number) {
  return getProblemTitle(id)
}

const initials = computed(() => {
  const name = profile.value?.username?.trim() || ''
  if (!name) return 'U'
  const s = name.replace(/[_\-]+/g, ' ')
  const parts = s.split(/\s+/).filter(Boolean)
  const pick = parts.length >= 2 ? (parts[0][0] + parts[1][0]) : parts[0].slice(0, 2)
  return pick.toUpperCase()
})

const filteredSubmissions = computed(() => {
  const list = profile.value?.submissions || []
  const q = query.value.trim().toLowerCase()
  if (!q) return list
  return list.filter((s) => {
    const hay = `${s.id} ${s.problem_id} ${s.language} ${s.status} ${s.submit_time}`.toLowerCase()
    return hay.includes(q)
  })
})

function getId(): number {
  const raw = route.params.id
  const id = typeof raw === 'string' ? Number(raw) : Number(raw?.[0])
  return Number.isFinite(id) ? id : NaN
}

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

function statusPillClass(status: string) {
  const s = (status || '').toLowerCase()
  if (s.includes('accept')) return 'pill-success'
  if (s.includes('wrong') || s.includes('wa')) return 'pill-danger'
  if (s.includes('tle') || s.includes('time')) return 'pill-warn'
  if (s.includes('mle') || s.includes('mem')) return 'pill-warn'
  if (s.includes('compile') || s.includes('ce')) return 'pill-info'
  if (s.includes('run') || s.includes('judg') || s.includes('submit')) return 'pill-info'
  return ''
}

async function load() {
  const id = getId()
  if (!Number.isFinite(id)) {
    error.value = '无效的用户 ID'
    profile.value = null
    return
  }

  loading.value = true
  error.value = ''
  try {
    profile.value = await apiGet<UserProfile>(`/api/user/${id}`, { auth: false })
    await ensureAllProblemTitlesLoaded().catch(() => void 0)
  } catch (e: any) {
    error.value = e?.message ?? String(e)
    profile.value = null
  } finally {
    loading.value = false
  }
}

onMounted(load)
</script>

<style scoped>
.profile-page {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.topbar {
  display: flex;
  align-items: center;
  justify-content: flex-start;
  margin-top: 2px;
}

.back {
  display: inline-flex;
  align-items: center;
  gap: 8px;
  padding: 6px 10px;
  border-radius: 12px;
  border: 1px solid rgba(15, 23, 42, 0.10);
  background: rgba(255, 255, 255, 0.7);
}

.hero {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 14px;
}

.hero-left {
  display: flex;
  align-items: center;
  gap: 14px;
  min-width: 0;
}

.avatar {
  width: 56px;
  height: 56px;
  border-radius: 16px;
  display: grid;
  place-items: center;
  font-weight: 800;
  letter-spacing: 0.5px;
  color: #0b1220;
  background: linear-gradient(135deg, rgba(79, 70, 229, 0.18), rgba(6, 118, 71, 0.14));
  border: 1px solid rgba(15, 23, 42, 0.10);
}

.hero-text {
  min-width: 0;
}

.title-row {
  display: flex;
  align-items: center;
  gap: 10px;
  flex-wrap: wrap;
}

.page-title {
  margin: 0;
  font-size: 24px;
  line-height: 1.15;
  letter-spacing: -0.01em;
}

.meta {
  margin-top: 4px;
  font-size: 13px;
}

.hero-actions {
  display: flex;
  align-items: center;
  gap: 10px;
}

.stats {
  display: grid;
  grid-template-columns: repeat(3, minmax(0, 1fr));
  gap: 12px;
}

.stat {
  padding: 14px;
}

.stat-label {
  font-size: 13px;
}

.stat-value {
  font-size: 24px;
  font-weight: 850;
  letter-spacing: -0.02em;
  margin-top: 6px;
}

.stat-hint {
  font-size: 12px;
  margin-top: 2px;
}

.section {
  padding: 14px;
}

.section-head {
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.section-title {
  margin: 0;
  font-size: 16px;
  letter-spacing: -0.01em;
}

.pill-list {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
  margin-top: 10px;
}

.empty {
  padding: 10px 0;
}

.tools {
  display: flex;
  align-items: center;
  gap: 10px;
}

.search {
  width: min(420px, 60vw);
}

@media (max-width: 860px) {
  .hero {
    flex-direction: column;
    align-items: stretch;
  }
  .hero-actions {
    justify-content: flex-end;
  }
  .stats {
    grid-template-columns: 1fr;
  }
  .search {
    width: 100%;
  }
}
</style>
