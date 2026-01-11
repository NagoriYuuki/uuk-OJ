import { apiGet } from './api'

export type UserLite = { id: number; username: string }
export type ProblemLite = { id: number; title: string }

const problemTitleCache = new Map<number, string>()
const userNameCache = new Map<number, string>()

let allProblemsLoaded = false
let loadingAllProblems: Promise<void> | null = null

export function getProblemTitle(problemId: number) {
  return problemTitleCache.get(problemId) || ''
}

export async function ensureAllProblemTitlesLoaded() {
  if (allProblemsLoaded) return
  if (loadingAllProblems) return loadingAllProblems

  loadingAllProblems = (async () => {
    const list = await apiGet<ProblemLite[]>('/api/problems', { auth: false })
    for (const p of list || []) {
      if (typeof p?.id === 'number' && typeof p?.title === 'string') {
        problemTitleCache.set(p.id, p.title)
      }
    }
    allProblemsLoaded = true
  })().finally(() => {
    loadingAllProblems = null
  })

  return loadingAllProblems
}

export function getUsername(userId: number) {
  return userNameCache.get(userId) || ''
}

export async function prefetchUsernames(userIds: number[], opts?: { limit?: number }) {
  const limit = Math.max(0, Math.min(opts?.limit ?? 30, 80))
  const uniq = Array.from(new Set(userIds.filter((x) => Number.isFinite(x))))
  const targets = uniq.filter((id) => !userNameCache.has(id)).slice(0, limit)

  await Promise.all(
    targets.map(async (id) => {
      try {
        // /api/user/:id returns a full profile; we only cache username here.
        const u = await apiGet<any>(`/api/user/${id}`, { auth: false })
        const name = typeof u?.username === 'string' ? u.username : ''
        if (name) userNameCache.set(id, name)
      } catch {
        // ignore
      }
    }),
  )
}
