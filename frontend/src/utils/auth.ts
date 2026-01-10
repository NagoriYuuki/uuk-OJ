import { reactive } from 'vue'
import { apiGet, apiPost } from './api'

export type UserInfo = {
  id: number
  role: number
}

export const authState = reactive({
  token: localStorage.getItem('token') || '',
  username: localStorage.getItem('username') || '',
  userId: (() => {
    const v = Number(localStorage.getItem('userId') || '')
    return Number.isFinite(v) && v > 0 ? v : null
  })() as number | null,
  role: (() => {
    const v = Number(localStorage.getItem('role') || '')
    return Number.isFinite(v) ? v : 0
  })(),
  loaded: false,
})

export function setSession(token: string, username?: string) {
  authState.token = token
  localStorage.setItem('token', token)
  if (username !== undefined) {
    authState.username = username
    localStorage.setItem('username', username)
  }
}

export function clearSession() {
  authState.token = ''
  authState.username = ''
  authState.userId = null
  authState.role = 0
  authState.loaded = false
  localStorage.removeItem('token')
  localStorage.removeItem('username')
  localStorage.removeItem('userId')
  localStorage.removeItem('role')
}

export async function refreshUserInfo(force = false) {
  if (!authState.token) {
    authState.loaded = true
    authState.userId = null
    authState.role = 0
    return null
  }
  if (authState.loaded && !force) return { id: authState.userId, role: authState.role } as any

  try {
    const info = await apiGet<UserInfo>('/api/auth/userinfo')
    authState.userId = info.id
    authState.role = info.role
    authState.loaded = true
    localStorage.setItem('userId', String(info.id))
    localStorage.setItem('role', String(info.role))
    return info
  } catch {
    clearSession()
    return null
  }
}

export async function login(username: string, password: string) {
  const data = await apiPost<{ token: string }>('/api/auth/login', { username, password }, { auth: false })
  setSession(data.token, username)
  await refreshUserInfo(true)
  return data
}
