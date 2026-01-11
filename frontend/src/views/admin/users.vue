<template>
    <div class="admin-container">
        <!-- 顶部 Header -->
        <div class="page-header">
            <div class="header-content">
                <h1 class="page-title">用户管理</h1>
                <p class="subtitle">管理注册用户、分配角色权限</p>
            </div>
            <div class="header-actions">
                <button class="btn btn-primary" @click="openCreate">
                    <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                        <path d="M16 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2" />
                        <circle cx="8.5" cy="7" r="4" />
                        <line x1="20" y1="8" x2="20" y2="14" />
                        <line x1="23" y1="11" x2="17" y2="11" />
                    </svg>
                    新建用户
                </button>
            </div>
        </div>

        <!-- 搜索栏 -->
        <div class="toolbar card">
            <div class="search-wrapper">
                <svg class="search-icon" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                    stroke-width="2">
                    <circle cx="11" cy="11" r="8"></circle>
                    <line x1="21" y1="21" x2="16.65" y2="16.65"></line>
                </svg>
                <input v-model="searchQuery" type="text" placeholder="搜索用户 ID 或用户名..." class="search-input">
            </div>
            <div class="toolbar-stats">
                共 {{ filteredUsers.length }} 位用户
            </div>
        </div>

        <!-- 数据表格 -->
        <div class="card table-card">
            <div v-if="loading" class="loading-state">
                <div class="spinner"></div> 加载中...
            </div>

            <table v-else class="data-table">
                <thead>
                    <tr>
                        <th width="80">ID</th>
                        <th>用户</th>
                        <th>角色</th>
                        <th width="150" style="text-align: right;">操作</th>
                    </tr>
                </thead>
                <tbody>
                    <tr v-for="u in filteredUsers" :key="u.id">
                        <td><span class="id-badge">#{{ u.id }}</span></td>
                        <td>
                            <div class="user-cell">
                                <div class="avatar" :style="{ backgroundColor: getAvatarColor(u.username) }">
                                    {{ u.username.charAt(0).toUpperCase() }}
                                </div>
                                <span class="username">{{ u.username }}</span>
                                <span v-if="u.id === currentUserId" class="me-badge">你自己</span>
                            </div>
                        </td>
                        <td>
                            <span :class="['role-badge', u.role === 1 ? 'admin' : 'user']">
                                {{ u.role === 1 ? '管理员' : '普通用户' }}
                            </span>
                        </td>
                        <td style="text-align: right;">
                            <div class="action-group">
                                <button class="btn-icon edit" @click="openEdit(u)" title="编辑">
                                    <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                                        stroke-width="2">
                                        <path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"></path>
                                        <path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"></path>
                                    </svg>
                                </button>
                                <button class="btn-icon delete" @click="deleteUser(u.id)"
                                    :disabled="u.id === currentUserId"
                                    :title="u.id === currentUserId ? '无法删除自己' : '删除'">
                                    <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                                        stroke-width="2">
                                        <polyline points="3 6 5 6 21 6"></polyline>
                                        <path
                                            d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2">
                                        </path>
                                    </svg>
                                </button>
                            </div>
                        </td>
                    </tr>
                    <tr v-if="filteredUsers.length === 0">
                        <td colspan="4" class="empty-state">未找到匹配的用户</td>
                    </tr>
                </tbody>
            </table>
        </div>

        <!-- 弹窗 (使用 Transition 动画) -->
        <Transition name="fade">
            <div v-if="showModal" class="modal-overlay" @click.self="closeModal">
                <div class="modal">
                    <div class="modal-header">
                        <h2>{{ isNew ? '新建用户' : '编辑用户' }}</h2>
                        <button class="close-btn" @click="closeModal">&times;</button>
                    </div>

                    <form @submit.prevent="submitForm">
                        <div class="form-group">
                            <label>用户名</label>
                            <input v-model="form.username" class="input-control" required placeholder="请输入用户名" />
                        </div>
                        <div class="form-group">
                            <label>密码</label>
                            <input v-model="form.password" type="password" class="input-control"
                                :placeholder="isNew ? '必填' : '留空则不修改'" :required="isNew" />
                            <small class="help-text" v-if="!isNew">仅需在重置密码时填写</small>
                        </div>
                        <div class="form-group">
                            <label>角色权限</label>
                            <div class="select-wrapper">
                                <select v-model.number="form.role" class="input-control">
                                    <option :value="0">普通用户 (User)</option>
                                    <option :value="1">管理员 (Admin)</option>
                                </select>
                            </div>
                        </div>

                        <div class="modal-footer">
                            <button type="button" class="btn btn-secondary" @click="closeModal">取消</button>
                            <button type="submit" class="btn btn-primary" :disabled="submitting">
                                <span v-if="submitting" class="spinner-sm"></span>
                                {{ submitting ? '保存中...' : '保存' }}
                            </button>
                        </div>
                    </form>
                </div>
            </div>
        </Transition>
    </div>
</template>

<script setup lang="ts">
import { ref, onMounted, reactive, computed } from 'vue'

interface User {
    id: number
    username: string
    role: number
}

const users = ref<User[]>([])
const loading = ref(false)
const searchQuery = ref('')
const showModal = ref(false)
const isNew = ref(false)
const currentUserId = ref<number | null>(null)
const submitting = ref(false)

const form = reactive({
    id: 0,
    username: '',
    password: '',
    role: 0
})

// 前端过滤
const filteredUsers = computed(() => {
    const q = searchQuery.value.trim().toLowerCase()
    if (!q) return users.value
    return users.value.filter(u =>
        u.username.toLowerCase().includes(q) ||
        u.id.toString().includes(q)
    )
})

// 根据用户名生成头像颜色
function getAvatarColor(username: string) {
    const colors = ['#f56a00', '#7265e6', '#ffbf00', '#00a2ae', '#10b981', '#ef4444', '#3b82f6']
    let hash = 0
    for (let i = 0; i < username.length; i++) {
        hash = username.charCodeAt(i) + ((hash << 5) - hash)
    }
    return colors[Math.abs(hash) % colors.length]
}

async function fetchUsers() {
    const token = localStorage.getItem('token')
    if (!token) return

    loading.value = true
    try {
        // 并发请求
        const [meRes, listRes] = await Promise.all([
            fetch('/api/auth/userinfo', { headers: { 'Authorization': `Bearer ${token}` } }),
            fetch('/api/admin/users', { headers: { 'Authorization': `Bearer ${token}` } })
        ])

        if (meRes.ok) {
            const meData = await meRes.json()
            currentUserId.value = meData.id
        }

        if (listRes.ok) {
            users.value = await listRes.json()
        }
    } catch (e) {
        console.error(e)
    } finally {
        loading.value = false
    }
}

function openCreate() {
    isNew.value = true
    form.id = 0
    form.username = ''
    form.password = ''
    form.role = 0
    showModal.value = true
}

function openEdit(u: User) {
    isNew.value = false
    form.id = u.id
    form.username = u.username
    form.password = ''
    form.role = u.role
    showModal.value = true
}

function closeModal() {
    showModal.value = false
}

async function submitForm() {
    const token = localStorage.getItem('token')
    if (!token) return alert('请先登录')

    submitting.value = true
    const url = isNew.value ? '/api/admin/users' : `/api/admin/users/${form.id}`
    const method = isNew.value ? 'POST' : 'PUT'

    const payload: any = {
        username: form.username,
        role: form.role
    }
    if (form.password) payload.password = form.password

    try {
        const res = await fetch(url, {
            method: method,
            headers: {
                'Content-Type': 'application/json',
                'Authorization': `Bearer ${token}`
            },
            body: JSON.stringify(payload)
        })

        if (res.ok) {
            closeModal()
            fetchUsers()
        } else {
            const data = await res.json().catch(() => ({}))
            alert('操作失败: ' + (data.message || res.statusText))
        }
    } catch (e) {
        alert('请求出错')
    } finally {
        submitting.value = false
    }
}

async function deleteUser(id: number) {
    if (!confirm('确定要删除该用户吗? 此操作不可恢复！')) return
    const token = localStorage.getItem('token')

    try {
        const res = await fetch(`/api/admin/users/${id}`, {
            method: 'DELETE',
            headers: { 'Authorization': `Bearer ${token}` }
        })
        if (res.ok) {
            // 前端直接移除
            users.value = users.value.filter(u => u.id !== id)
        } else {
            const data = await res.json().catch(() => ({}))
            alert('删除失败: ' + (data.message || 'Error'))
        }
    } catch (e) {
        alert('请求出错')
    }
}

onMounted(fetchUsers)
</script>

<style scoped>
/* 变量定义 */
:root {
    --primary: #4f46e5;
    --primary-hover: #4338ca;
    --text-main: #1e293b;
    --text-muted: #64748b;
    --border: #e2e8f0;
    --bg-page: #f8fafc;
    --danger: #ef4444;
}

.admin-container {
    max-width: 1000px;
    margin: 0 auto;
    padding: 2rem 1rem;
    color: var(--text-main);
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
}

/* Header */
.page-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 2rem;
}

.page-title {
    font-size: 1.8rem;
    font-weight: 700;
    color: #0f172a;
    margin: 0;
}

.subtitle {
    margin: 4px 0 0 0;
    color: var(--text-muted);
    font-size: 0.9rem;
}

/* 按钮 */
.btn {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    gap: 6px;
    padding: 10px 16px;
    border-radius: 8px;
    font-weight: 600;
    border: none;
    cursor: pointer;
    transition: all 0.2s;
    font-size: 0.9rem;
}

.btn-primary {
    background: var(--primary);
    color: white;
    box-shadow: 0 4px 6px -1px rgba(79, 70, 229, 0.2);
}

.btn-primary:hover {
    background: var(--primary-hover);
    transform: translateY(-1px);
}

.btn-secondary {
    background: white;
    border: 1px solid var(--border);
    color: var(--text-main);
}

.btn-secondary:hover {
    background: #f8fafc;
    border-color: #cbd5e1;
}

/* 搜索栏 */
.toolbar {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 1.5rem;
    padding: 1rem;
}

.search-wrapper {
    position: relative;
    width: 300px;
}

.search-icon {
    position: absolute;
    left: 10px;
    top: 50%;
    transform: translateY(-50%);
    color: var(--text-muted);
    pointer-events: none;
}

.search-input {
    width: 100%;
    padding: 8px 10px 8px 36px;
    border: 1px solid var(--border);
    border-radius: 6px;
    font-size: 0.9rem;
    transition: all 0.2s;
}

.search-input:focus {
    border-color: var(--primary);
    outline: none;
    box-shadow: 0 0 0 3px rgba(79, 70, 229, 0.1);
}

.toolbar-stats {
    font-size: 0.9rem;
    color: var(--text-muted);
    font-weight: 500;
}

/* 卡片与表格 */
.card {
    background: white;
    border: 1px solid var(--border);
    border-radius: 12px;
    box-shadow: 0 1px 2px 0 rgba(0, 0, 0, 0.05);
}

.table-card {
    overflow: hidden;
}

.data-table {
    width: 100%;
    border-collapse: collapse;
    text-align: left;
}

.data-table th {
    background: #f8fafc;
    padding: 12px 16px;
    font-size: 0.8rem;
    font-weight: 600;
    color: var(--text-muted);
    text-transform: uppercase;
    border-bottom: 1px solid var(--border);
}

.data-table td {
    padding: 12px 16px;
    border-bottom: 1px solid #f1f5f9;
    font-size: 0.95rem;
    vertical-align: middle;
}

.data-table tr:last-child td {
    border-bottom: none;
}

.data-table tr:hover {
    background: #f8fafc;
}

/* 表格内容组件 */
.id-badge {
    font-family: 'Monaco', monospace;
    font-size: 0.85rem;
    background: #f1f5f9;
    padding: 2px 6px;
    border-radius: 4px;
    color: var(--text-muted);
}

.user-cell {
    display: flex;
    align-items: center;
    gap: 10px;
}

.avatar {
    width: 32px;
    height: 32px;
    border-radius: 50%;
    color: white;
    display: flex;
    align-items: center;
    justify-content: center;
    font-weight: bold;
    font-size: 0.9rem;
    text-shadow: 0 1px 2px rgba(0, 0, 0, 0.1);
}

.username {
    font-weight: 600;
    color: #334155;
}

.me-badge {
    font-size: 0.75rem;
    background: #ecfdf5;
    color: #047857;
    padding: 2px 6px;
    border-radius: 10px;
    border: 1px solid #a7f3d0;
    margin-left: 5px;
}

.role-badge {
    display: inline-block;
    padding: 2px 8px;
    border-radius: 12px;
    font-size: 0.75rem;
    font-weight: 600;
}

.role-badge.admin {
    background: #fee2e2;
    color: #ef4444;
}

.role-badge.user {
    background: #eff6ff;
    color: #3b82f6;
}

/* 操作按钮 */
.action-group {
    display: flex;
    justify-content: flex-end;
    gap: 8px;
}

.btn-icon {
    background: none;
    border: none;
    padding: 6px;
    border-radius: 4px;
    cursor: pointer;
    color: var(--text-muted);
    transition: all 0.2s;
}

.btn-icon:hover {
    background: #e2e8f0;
}

.btn-icon.edit:hover {
    color: var(--primary);
    background: #e0e7ff;
}

.btn-icon.delete:hover:not(:disabled) {
    color: var(--danger);
    background: #fee2e2;
}

.btn-icon:disabled {
    opacity: 0.3;
    cursor: not-allowed;
}

/* 模态框 */
.modal-overlay {
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background: rgba(0, 0, 0, 0.4);
    backdrop-filter: blur(2px);
    display: flex;
    justify-content: center;
    align-items: center;
    z-index: 100;
}

.modal {
    background: white;
    padding: 24px;
    width: 420px;
    border-radius: 16px;
    box-shadow: 0 20px 25px -5px rgba(0, 0, 0, 0.1), 0 10px 10px -5px rgba(0, 0, 0, 0.04);
    animation: slideUp 0.3s cubic-bezier(0.16, 1, 0.3, 1);
}

.modal-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 1.5rem;
}

.modal-header h2 {
    margin: 0;
    font-size: 1.25rem;
    color: #0f172a;
}

.close-btn {
    background: none;
    border: none;
    font-size: 1.5rem;
    color: var(--text-muted);
    cursor: pointer;
    padding: 0;
    line-height: 1;
}

.close-btn:hover {
    color: var(--text-main);
}

.form-group {
    margin-bottom: 1.2rem;
}

.form-group label {
    display: block;
    font-size: 0.9rem;
    font-weight: 500;
    margin-bottom: 6px;
    color: #334155;
}

.input-control {
    width: 100%;
    padding: 10px;
    border: 1px solid var(--border);
    border-radius: 8px;
    font-size: 0.95rem;
    box-sizing: border-box;
    transition: all 0.2s;
}

.input-control:focus {
    border-color: var(--primary);
    outline: none;
    box-shadow: 0 0 0 3px rgba(79, 70, 229, 0.1);
}

.help-text {
    font-size: 0.8rem;
    color: var(--text-muted);
    margin-top: 4px;
    display: block;
}

.modal-footer {
    margin-top: 2rem;
    display: flex;
    justify-content: flex-end;
    gap: 10px;
}

/* 动画与状态 */
.fade-enter-active,
.fade-leave-active {
    transition: opacity 0.2s;
}

.fade-enter-from,
.fade-leave-to {
    opacity: 0;
}

@keyframes slideUp {
    from {
        transform: translateY(20px);
        opacity: 0;
    }

    to {
        transform: translateY(0);
        opacity: 1;
    }
}

.loading-state,
.empty-state {
    text-align: center;
    padding: 3rem;
    color: var(--text-muted);
}

.spinner {
    width: 20px;
    height: 20px;
    border: 2px solid #e2e8f0;
    border-top-color: var(--primary);
    border-radius: 50%;
    animation: spin 1s linear infinite;
    display: inline-block;
    vertical-align: middle;
}

.spinner-sm {
    width: 14px;
    height: 14px;
    border-width: 2px;
}

@keyframes spin {
    to {
        transform: rotate(360deg);
    }
}

@media (max-width: 640px) {
    .page-header {
        flex-direction: column;
        align-items: flex-start;
        gap: 1rem;
    }

    .toolbar {
        flex-direction: column;
        align-items: stretch;
        gap: 10px;
    }

    .search-wrapper {
        width: 100%;
    }

    .modal {
        width: 90%;
        margin: 0 20px;
    }
}
</style>