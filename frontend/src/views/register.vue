<template>
    <div class="register-container">
        <div class="register-card">
            <!-- 头部 -->
            <div class="card-header">
                <div class="logo-icon">🚀</div>
                <h1 class="title">加入社区</h1>
                <p class="subtitle">创建一个账号，记录你的 AC 之路</p>
            </div>

            <!-- 错误提示框 (带动画) -->
            <transition name="fade">
                <div v-if="error" class="error-alert">
                    <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                        <circle cx="12" cy="12" r="10" />
                        <line x1="12" y1="8" x2="12" y2="12" />
                        <line x1="12" y1="16" x2="12.01" y2="16" />
                    </svg>
                    <span>{{ error }}</span>
                </div>
            </transition>

            <!-- 表单 -->
            <form @submit.prevent="handleRegister" class="register-form">
                <div class="form-group">
                    <label>用户名</label>
                    <div class="input-wrapper">
                        <svg class="input-icon" width="18" height="18" viewBox="0 0 24 24" fill="none"
                            stroke="currentColor" stroke-width="2">
                            <path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2" />
                            <circle cx="12" cy="7" r="4" />
                        </svg>
                        <input v-model="username" type="text" required placeholder="设置一个响亮的昵称" class="form-input" />
                    </div>
                </div>

                <div class="form-group">
                    <label>密码</label>
                    <div class="input-wrapper">
                        <svg class="input-icon" width="18" height="18" viewBox="0 0 24 24" fill="none"
                            stroke="currentColor" stroke-width="2">
                            <rect x="3" y="11" width="18" height="11" rx="2" ry="2" />
                            <path d="M7 11V7a5 5 0 0 1 10 0v4" />
                        </svg>
                        <input v-model="password" type="password" required placeholder="至少 6 位字符" class="form-input" />
                    </div>
                </div>

                <div class="form-group">
                    <label>确认密码</label>
                    <div class="input-wrapper">
                        <!-- 使用盾牌图标代表确认安全 -->
                        <svg class="input-icon" width="18" height="18" viewBox="0 0 24 24" fill="none"
                            stroke="currentColor" stroke-width="2">
                            <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z" />
                        </svg>
                        <input v-model="repassword" type="password" required placeholder="请再次输入密码" class="form-input" />
                    </div>
                </div>

                <button type="submit" class="submit-btn" :disabled="loading">
                    <span v-if="loading" class="spinner"></span>
                    {{ loading ? '注册中...' : '立即注册' }}
                </button>
            </form>

            <!-- 底部链接 -->
            <div class="card-footer">
                <p>已有账号？ <RouterLink to="/login" class="link">直接登录</RouterLink>
                </p>
                <div class="divider"></div>
                <RouterLink to="/" class="home-link">
                    <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                        <path d="M19 12H5m7 7l-7-7 7-7" />
                    </svg>
                    返回首页
                </RouterLink>
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'
import { useRouter } from 'vue-router'

const username = ref('')
const password = ref('')
const repassword = ref('')
const loading = ref(false)
const error = ref('')
const router = useRouter()

async function handleRegister() {
    loading.value = true
    error.value = ''

    // 前端简单校验
    if (password.value.length < 6) {
        error.value = '密码长度不能少于 6 位'
        loading.value = false
        return
    }

    if (password.value !== repassword.value) {
        error.value = '两次输入的密码不一致！'
        loading.value = false
        return
    }

    try {
        const resp = await fetch('/api/auth/register', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username: username.value, password: password.value })
        })

        const data = await resp.json().catch(() => null)

        if (!resp.ok)
            throw new Error(data?.message || '注册失败')

        // 可以换成 Toast 提示，这里暂时保留 alert 或自定义弹窗
        // alert('注册成功！') 
        // 建议直接跳转登录页，用户体验更流畅
        router.push({ name: 'login', query: { registered: 'true' } })
    } catch (e: any) {
        error.value = e.message
    } finally {
        loading.value = false
    }
}
</script>

<style scoped>
/* 变量定义 - 与登录页保持一致 */
:root {
    --primary: #4f46e5;
    --primary-hover: #4338ca;
    --text-main: #1e293b;
    --text-muted: #64748b;
    --border: #e2e8f0;
    --bg-gradient: linear-gradient(135deg, #f8fafc 0%, #e0e7ff 100%);
}

.register-container {
    min-height: 100vh;
    display: flex;
    justify-content: center;
    align-items: center;
    background: var(--bg-gradient);
    padding: 1rem;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
}

.register-card {
    background: white;
    width: 100%;
    max-width: 400px;
    padding: 2.5rem;
    border-radius: 16px;
    box-shadow: 0 10px 25px -5px rgba(0, 0, 0, 0.1), 0 8px 10px -6px rgba(0, 0, 0, 0.1);
    animation: slideUp 0.5s cubic-bezier(0.16, 1, 0.3, 1);
}

/* Header */
.card-header {
    text-align: center;
    margin-bottom: 2rem;
}

.logo-icon {
    font-size: 2.5rem;
    margin-bottom: 0.5rem;
    display: inline-block;
}

.title {
    font-size: 1.75rem;
    font-weight: 800;
    color: #0f172a;
    margin: 0 0 0.5rem 0;
}

.subtitle {
    color: var(--text-muted);
    font-size: 0.95rem;
    margin: 0;
}

/* Alert */
.error-alert {
    background: #fef2f2;
    border: 1px solid #fee2e2;
    color: #ef4444;
    padding: 0.75rem 1rem;
    border-radius: 8px;
    font-size: 0.9rem;
    display: flex;
    align-items: center;
    gap: 8px;
    margin-bottom: 1.5rem;
}

/* Form */
.register-form {
    display: flex;
    flex-direction: column;
    gap: 1.25rem;
}

.form-group label {
    display: block;
    font-size: 0.9rem;
    font-weight: 500;
    color: #334155;
    margin-bottom: 0.5rem;
}

.input-wrapper {
    position: relative;
    display: flex;
    align-items: center;
}

.input-icon {
    position: absolute;
    left: 12px;
    color: #94a3b8;
    pointer-events: none;
}

.form-input {
    width: 100%;
    padding: 10px 12px 10px 40px;
    border: 1px solid var(--border);
    border-radius: 8px;
    font-size: 0.95rem;
    transition: all 0.2s;
    background: #f8fafc;
    box-sizing: border-box;
    /* 关键：防止 padding 撑大宽度 */
}

.form-input:focus {
    background: white;
    border-color: var(--primary);
    outline: none;
    box-shadow: 0 0 0 3px rgba(79, 70, 229, 0.1);
}

.submit-btn {
    margin-top: 0.5rem;
    width: 100%;
    padding: 12px;
    background: var(--primary);
    color: white;
    border: none;
    border-radius: 8px;
    font-weight: 600;
    font-size: 1rem;
    cursor: pointer;
    transition: all 0.2s;
    display: flex;
    justify-content: center;
    align-items: center;
    gap: 8px;
}

.submit-btn:hover:not(:disabled) {
    background: var(--primary-hover);
    transform: translateY(-1px);
    box-shadow: 0 4px 6px -1px rgba(79, 70, 229, 0.2);
}

.submit-btn:disabled {
    opacity: 0.7;
    cursor: not-allowed;
}

/* Footer */
.card-footer {
    margin-top: 2rem;
    text-align: center;
    font-size: 0.9rem;
    color: var(--text-muted);
}

.link {
    color: var(--primary);
    text-decoration: none;
    font-weight: 600;
}

.link:hover {
    text-decoration: underline;
}

.divider {
    height: 1px;
    background: var(--border);
    margin: 1.5rem 0;
}

.home-link {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    color: var(--text-muted);
    text-decoration: none;
    font-size: 0.85rem;
    transition: color 0.2s;
}

.home-link:hover {
    color: var(--text-main);
}

/* Animation */
.spinner {
    width: 16px;
    height: 16px;
    border: 2px solid rgba(255, 255, 255, 0.3);
    border-top-color: white;
    border-radius: 50%;
    animation: spin 0.8s linear infinite;
}

@keyframes spin {
    to {
        transform: rotate(360deg);
    }
}

@keyframes slideUp {
    from {
        opacity: 0;
        transform: translateY(20px);
    }

    to {
        opacity: 1;
        transform: translateY(0);
    }
}

.fade-enter-active,
.fade-leave-active {
    transition: opacity 0.3s;
}

.fade-enter-from,
.fade-leave-to {
    opacity: 0;
}
</style>