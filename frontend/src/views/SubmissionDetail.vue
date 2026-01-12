<template>
  <div>
    <p class="muted" style="margin: 6px 0 10px;">
      <RouterLink to="/submissions">← 返回提交列表</RouterLink>
    </p>

    <div style="display:flex; align-items:center; justify-content:space-between; gap: 12px; flex-wrap:wrap;">
      <h1 style="margin: 6px 0 10px;">提交 #{{ id }}</h1>
      <div style="display:flex; gap: 10px; align-items:center;">
        <!-- <label style="display:flex; gap:6px; align-items:center;" class="muted">
          <input type="checkbox" v-model="autoRefresh" />
          自动刷新
        </label>
        <button @click="load" :disabled="loading">{{ loading ? '加载中…' : '刷新' }}</button> -->
      </div>
    </div>

    <p v-if="error" class="error">{{ error }}</p>

    <div v-if="sub" class="card" style="margin-bottom: 12px;">
      <div style="display:flex; align-items:center; justify-content:space-between; gap: 12px; flex-wrap:wrap;">
        <div style="display:flex; align-items:center; gap: 10px; flex-wrap:wrap;">
          <span class="muted">进度</span>
          <span class="pill" :class="statusPillClass(sub.status)">{{ sub.status }}</span>
          <span v-if="isInProgress" class="muted" style="display:inline-flex; align-items:center; gap: 8px;">
            <span class="spinner" aria-hidden="true"></span>
            判题中…
          </span>
        </div>
        <div class="muted" style="font-size: 12px;">{{ progressLabel }}</div>
      </div>

      <div class="progress" style="margin-top: 10px;">
        <div class="progress-bar" :class="{ indeterminate: isInProgress }" :style="{ width: progressPercent + '%' }"></div>
        <div class="ticks" aria-hidden="true">
          <span class="tick" :style="{ left: tickWait + '%' }"></span>
          <span class="tick" :style="{ left: tickCompile + '%' }"></span>
          <span class="tick" :style="{ left: tickRunEnd + '%' }"></span>
        </div>
      </div>

      <div class="steps" style="margin-top: 10px;">
        <div class="step small" :class="stepClass(1)">
          <div class="dot"></div>
          <div class="label">等待</div>
        </div>
        <div class="step small" :class="stepClass(2)">
          <div class="dot"></div>
          <div class="label">编译</div>
        </div>
        <div class="step" :class="stepClass(3)">
          <div class="dot"></div>
          <div class="label">运行/结果</div>
        </div>
      </div>

      <div v-if="isDone" class="final">
        <div class="final-label muted">最终结果</div>
        <div class="final-status" :class="finalClass">{{ sub.status }}</div>
        <div v-if="sub.detail" class="final-detail muted">{{ sub.detail }}</div>
      </div>
    </div>

    <div v-if="sub" class="card" style="display:grid; grid-template-columns: 1fr 1fr; gap: 10px;">
      <div>
        <b>题目</b>：
        <RouterLink :to="{ name: 'problem-detail', params: { id: sub.problem_id } }">
          #{{ sub.problem_id }}
          <span v-if="problemTitle" class="muted"> {{ problemTitle }}</span>
        </RouterLink>
      </div>
      <div>
        <b>用户</b>：
        <RouterLink :to="{ name: 'user-profile', params: { id: sub.user_id } }">{{ username || '用户' }}</RouterLink>
      </div>
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
import { computed, onBeforeUnmount, onMounted, ref, watch } from 'vue'
import { RouterLink } from 'vue-router'
import { apiGet } from '../utils/api'
import { ensureAllProblemTitlesLoaded, getProblemTitle, getUsername, prefetchUsernames } from '../utils/lookup'

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

const problemTitle = computed(() => {
  if (!sub.value) return ''
  return getProblemTitle(sub.value.problem_id)
})

const username = computed(() => {
  if (!sub.value) return ''
  return getUsername(sub.value.user_id)
})

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
  return s.includes('submit') || s.includes('running') || s.includes('judg') || s.includes('pending') || s.includes('compil')
}

const isInProgress = computed(() => {
  if (!sub.value) return false
  return shouldKeepRefreshing(sub.value.status)
})

const isDone = computed(() => {
  if (!sub.value) return false
  return !shouldKeepRefreshing(sub.value.status)
})

const stage = computed(() => {
  if (!sub.value) return 'waiting'
  const s = (sub.value.status || '').toLowerCase()
  const d = (sub.value.detail || '').toLowerCase()

  // Prefer detail-driven inference when available (more granular).
  if (d.includes('running on test') || d.includes('test #') || d.includes('test#')) return 'running'
  if (s.includes('running') || s.includes('judg')) return 'running'
  if (s.includes('compil') || d.includes('compil')) return 'compiling'
  if (s.includes('submit') || s.includes('pending')) return 'waiting'
  return 'done'
})

const testProgress = computed(() => {
  const detail = String(sub.value?.detail || '')
  const m = detail.match(/test\s*#\s*(\d+)(?:\s*\/\s*(\d+))?/i)
  const cur = m ? Number(m[1]) : NaN
  const total = m && m[2] ? Number(m[2]) : NaN
  return {
    current: Number.isFinite(cur) ? cur : null,
    total: Number.isFinite(total) ? total : null,
  }
})

const WEIGHT_WAIT = 8
const WEIGHT_COMPILE = 12
const WEIGHT_RUN = 60
const WEIGHT_DONE = 20

const tickWait = computed(() => WEIGHT_WAIT)
const tickCompile = computed(() => WEIGHT_WAIT + WEIGHT_COMPILE)
const tickRunEnd = computed(() => WEIGHT_WAIT + WEIGHT_COMPILE + WEIGHT_RUN)

const progressPercent = computed(() => {
  if (!sub.value) return 0
  if (stage.value === 'waiting') return Math.max(3, WEIGHT_WAIT * 0.7)
  if (stage.value === 'compiling') return WEIGHT_WAIT + WEIGHT_COMPILE * 0.65
  if (stage.value === 'running') {
    const { current, total } = testProgress.value
    if (current && total && total > 0) {
      const frac = Math.min(1, Math.max(0, current / total))
      return WEIGHT_WAIT + WEIGHT_COMPILE + WEIGHT_RUN * frac
    }
    return WEIGHT_WAIT + WEIGHT_COMPILE + WEIGHT_RUN * 0.25
  }
  return 100
})

const progressLabel = computed(() => {
  if (!sub.value) return ''
  if (stage.value === 'waiting') return '已提交，等待判题机接单'
  if (stage.value === 'compiling') return '正在编译'
  if (stage.value === 'running') {
    const { current, total } = testProgress.value
    if (current && total) return `运行测试点 ${current}/${total}`
    if (current) return `运行测试点 ${current}`
    return '正在运行测试点'
  }
  return '判题已结束'
})

function stepClass(n: number) {
  const stageToIndex = stage.value === 'waiting' ? 1 : stage.value === 'compiling' ? 2 : stage.value === 'done' ? 4 : 3
  return {
    done: stageToIndex > n,
    active: stageToIndex === n,
  }
}

const finalClass = computed(() => {
  if (!sub.value) return ''
  const s = (sub.value.status || '').toLowerCase()
  if (s.includes('accept')) return 'ok'
  if (s.includes('wrong') || s.includes('wa')) return 'bad'
  if (s.includes('tle') || s.includes('time')) return 'warn'
  if (s.includes('mle') || s.includes('mem')) return 'warn'
  if (s.includes('compile') || s.includes('ce')) return 'bad'
  if (s.includes('system')) return 'bad'
  return 'neutral'
})

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
  loading.value = true
  error.value = ''
  try {
    sub.value = await apiGet<SubmissionDetail>(`/api/submit/${id}`)
    await ensureAllProblemTitlesLoaded().catch(() => void 0)
    if (sub.value) {
      await prefetchUsernames([sub.value.user_id], { limit: 1 }).catch(() => void 0)
    }
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
    if (loading.value) return
    if (!sub.value) {
      await load()
      return
    }
    if (shouldKeepRefreshing(sub.value.status)) await load()
  }, 100)
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

<style scoped>
.progress {
  position: relative;
  height: 10px;
  border-radius: 999px;
  background: rgba(15, 23, 42, 0.08);
  overflow: hidden;
}

.ticks {
  position: absolute;
  inset: 0;
  pointer-events: none;
}

.tick {
  position: absolute;
  top: -2px;
  bottom: -2px;
  width: 1px;
  background: rgba(15, 23, 42, 0.18);
}

.progress-bar {
  height: 100%;
  border-radius: 999px;
  background: linear-gradient(90deg, rgba(79, 70, 229, 0.85), rgba(6, 118, 71, 0.85));
  transition: width 240ms ease;
}

.progress-bar.indeterminate {
  position: relative;
}

.progress-bar.indeterminate::after {
  content: '';
  position: absolute;
  inset: 0;
  background: linear-gradient(
    90deg,
    rgba(255, 255, 255, 0.00),
    rgba(255, 255, 255, 0.35),
    rgba(255, 255, 255, 0.00)
  );
  transform: translateX(-60%);
  animation: shimmer 1200ms ease-in-out infinite;
}

.steps {
  display: grid;
  grid-template-columns: repeat(3, minmax(0, 1fr));
  gap: 10px;
}

.step {
  display: flex;
  align-items: center;
  gap: 8px;
  color: rgba(15, 23, 42, 0.70);
}

.step.small .label {
  font-size: 12px;
}

.step.small {
  opacity: 0.85;
}

.dot {
  width: 10px;
  height: 10px;
  border-radius: 999px;
  background: rgba(15, 23, 42, 0.20);
  border: 1px solid rgba(15, 23, 42, 0.14);
}

.label {
  font-size: 13px;
}

.step.done {
  color: rgba(6, 118, 71, 0.90);
}
.step.done .dot {
  background: rgba(6, 118, 71, 0.75);
  border-color: rgba(6, 118, 71, 0.35);
}

.step.active {
  color: rgba(79, 70, 229, 0.95);
}
.step.active .dot {
  background: rgba(79, 70, 229, 0.80);
  border-color: rgba(79, 70, 229, 0.35);
}

.spinner {
  width: 12px;
  height: 12px;
  border-radius: 999px;
  border: 2px solid rgba(15, 23, 42, 0.18);
  border-top-color: rgba(79, 70, 229, 0.85);
  animation: spin 900ms linear infinite;
}

@keyframes spin {
  to {
    transform: rotate(360deg);
  }
}

@keyframes shimmer {
  to {
    transform: translateX(60%);
  }
}

.final {
  margin-top: 12px;
  padding-top: 12px;
  border-top: 1px solid rgba(15, 23, 42, 0.10);
}

.final-label {
  font-size: 12px;
}

.final-status {
  margin-top: 6px;
  font-size: 22px;
  font-weight: 900;
  letter-spacing: -0.02em;
}

.final-status.ok {
  color: rgba(6, 118, 71, 0.95);
}
.final-status.warn {
  color: rgba(181, 71, 8, 0.95);
}
.final-status.bad {
  color: rgba(180, 35, 24, 0.95);
}
.final-status.neutral {
  color: rgba(15, 23, 42, 0.92);
}

.final-detail {
  margin-top: 6px;
  font-size: 13px;
}
</style>
