// 1. 在这里定义 BASE_URL
// 如果你使用 Vite 代理（方案一），这里留空字符串 '' 即可
// 浏览器会自动使用相对路径，Vite 会帮你转发到 18080
const BASE_URL = '';

// 如果不使用代理，必须写死后端地址（会有跨域问题，不推荐）
// const BASE_URL = 'http://localhost:18080';

type RequestOptions = {
  method?: string
  headers?: Record<string, string>
  body?: any
  auth?: boolean
  rawBody?: boolean
}

async function parseBody(resp: Response) {
  const ct = resp.headers.get('content-type') || ''
  if (ct.includes('application/json')) {
    return await resp.json().catch(() => null)
  }
  return await resp.text().catch(() => '')
}

export async function apiRequest<T = any>(path: string, options: RequestOptions = {}): Promise<T> {
  const method = options.method || 'GET'
  const headers: Record<string, string> = { ...(options.headers || {}) }

  if (options.auth !== false) {
    const token = localStorage.getItem('token')
    if (token && !headers.Authorization) headers.Authorization = `Bearer ${token}`
  }

  let body: any = undefined
  if (options.body !== undefined) {
    if (options.rawBody) {
      body = options.body
    } else {
      headers['Content-Type'] = headers['Content-Type'] || 'application/json'
      body = typeof options.body === 'string' ? options.body : JSON.stringify(options.body)
    }
  }

  // 2. 修改这里：拼接 URL
  // 如果 path 本身是 http 开头的（比如外部链接），就不拼接 BASE_URL
  const url = path.startsWith('http') ? path : BASE_URL + path;

  const resp = await fetch(url, { method, headers, body })
  const data = await parseBody(resp)

  if (!resp.ok) {
    const msg =
      (data && typeof data === 'object' && 'message' in data && (data as any).message) ||
      (typeof data === 'string' && data) ||
      `HTTP ${resp.status}`
    throw new Error(String(msg))
  }
  return data as T
}

export function apiGet<T = any>(path: string, opts: RequestOptions = {}) {
  return apiRequest<T>(path, { ...opts, method: 'GET' })
}

export function apiPost<T = any>(path: string, body?: any, opts: RequestOptions = {}) {
  return apiRequest<T>(path, { ...opts, method: 'POST', body })
}

export function apiPut<T = any>(path: string, body?: any, opts: RequestOptions = {}) {
  return apiRequest<T>(path, { ...opts, method: 'PUT', body })
}

export function apiDelete<T = any>(path: string, opts: RequestOptions = {}) {
  return apiRequest<T>(path, { ...opts, method: 'DELETE' })
}