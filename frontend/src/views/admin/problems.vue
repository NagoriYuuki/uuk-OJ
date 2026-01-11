<template>
    <div class="admin-container">
        <!-- 顶部 Header -->
        <div class="page-header">
            <div class="header-content">
                <h1 class="page-title">题目管理</h1>
                <p class="subtitle">管理题库内容、编辑限制与测试数据</p>
            </div>
            <div class="header-actions">
                <button class="btn btn-primary" @click="$router.push({ name: 'problem-create' })">
                    <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                        <line x1="12" y1="5" x2="12" y2="19"></line>
                        <line x1="5" y1="12" x2="19" y2="12"></line>
                    </svg>
                    新建题目
                </button>
            </div>
        </div>

        <!-- 过滤与搜索栏 -->
        <div class="toolbar card">
            <div class="search-wrapper">
                <svg class="search-icon" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                    stroke-width="2">
                    <circle cx="11" cy="11" r="8"></circle>
                    <line x1="21" y1="21" x2="16.65" y2="16.65"></line>
                </svg>
                <input v-model="searchQuery" type="text" placeholder="搜索题目 ID 或标题..." class="search-input">
            </div>
            <div class="toolbar-stats">
                共 {{ filteredProblems.length }} 题
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
                        <th>标题</th>
                        <th width="200">数据统计 (AC / Total)</th>
                        <th width="150" style="text-align: right;">操作</th>
                    </tr>
                </thead>
                <tbody>
                    <tr v-for="p in filteredProblems" :key="p.id">
                        <td>
                            <span class="id-badge">#{{ p.id }}</span>
                        </td>
                        <td>
                            <div class="problem-title">{{ p.title }}</div>
                        </td>
                        <td>
                            <div class="stats-cell">
                                <div class="stats-text">
                                    <span class="ac-text">{{ p.ac_count }}</span>
                                    <span class="divider">/</span>
                                    <span class="total-text">{{ p.sub_count }}</span>
                                </div>
                                <div class="progress-bg">
                                    <div class="progress-fill"
                                        :style="{ width: getPassRate(p) + '%', backgroundColor: getRateColor(getPassRate(p)) }">
                                    </div>
                                </div>
                            </div>
                        </td>
                        <td style="text-align: right;">
                            <div class="action-group">
                                <button class="btn-icon edit"
                                    @click="$router.push({ name: 'problem-edit', params: { id: p.id } })" title="编辑">
                                    <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                                        stroke-width="2">
                                        <path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"></path>
                                        <path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"></path>
                                    </svg>
                                </button>
                                <button class="btn-icon delete" @click="deleteProblem(p.id)" title="删除">
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
                    <tr v-if="filteredProblems.length === 0">
                        <td colspan="4" class="empty-state">
                            暂无符合条件的题目
                        </td>
                    </tr>
                </tbody>
            </table>
        </div>
    </div>
</template>

<script setup lang="ts">
import { ref, onMounted, computed } from 'vue'
import { apiGet } from '../../utils/api'; // 假设你有封装好的 apiGet

interface Problem {
    id: number;
    title: string;
    ac_count: number;
    sub_count: number;
}

const problems = ref<Problem[]>([])
const loading = ref(false)
const searchQuery = ref('')

// 计算属性：前端搜索过滤
const filteredProblems = computed(() => {
    const q = searchQuery.value.trim().toLowerCase()
    if (!q) return problems.value
    return problems.value.filter(p =>
        p.id.toString().includes(q) ||
        p.title.toLowerCase().includes(q)
    )
})

async function getProblems() {
    loading.value = true
    try {
        // 这里使用你的 apiGet 或者 fetch 都可以
        const res = await fetch('/api/problems')
        if (res.ok) {
            problems.value = await res.json()
        }
    } catch (e) {
        console.error(e)
    } finally {
        loading.value = false
    }
}

async function deleteProblem(id: number) {
    if (!confirm(`确定要彻底删除题目 #${id} 吗？\n该操作不可恢复！`)) return

    const token = localStorage.getItem('token')
    try {
        const res = await fetch(`/api/admin/problems/${id}`, {
            method: 'DELETE',
            headers: { 'Authorization': `Bearer ${token}` }
        })

        if (res.ok) {
            // 前端直接移除，减少一次请求
            problems.value = problems.value.filter(p => p.id !== id)
        } else {
            const data = await res.json().catch(() => ({}))
            alert('删除失败: ' + (data.message || 'Unknown error'))
        }
    } catch (e) {
        alert('网络错误')
    }
}

function getPassRate(p: Problem) {
    if (!p.sub_count) return 0
    return Math.round((p.ac_count / p.sub_count) * 100)
}

function getRateColor(rate: number) {
    if (rate >= 60) return '#10b981'
    if (rate >= 30) return '#f59e0b'
    return '#ef4444'
}

onMounted(getProblems)
</script>

<style scoped>
/* 全局变量 */
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

/* 按钮通用 */
.btn {
    display: inline-flex;
    align-items: center;
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

/* 工具栏 & 搜索 */
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

/* 卡片样式 */
.card {
    background: white;
    border: 1px solid var(--border);
    border-radius: 12px;
    box-shadow: 0 1px 2px 0 rgba(0, 0, 0, 0.05);
}

.table-card {
    overflow: hidden;
    /* 圆角溢出隐藏 */
}

/* 表格样式 */
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

/* 表格内容细节 */
.id-badge {
    font-family: 'Monaco', monospace;
    font-size: 0.85rem;
    background: #f1f5f9;
    padding: 2px 6px;
    border-radius: 4px;
    color: var(--text-muted);
}

.problem-title {
    font-weight: 600;
    color: #334155;
}

/* 进度条样式 */
.stats-cell {
    display: flex;
    flex-direction: column;
    gap: 4px;
}

.stats-text {
    font-size: 0.8rem;
    display: flex;
    gap: 2px;
}

.ac-text {
    color: #10b981;
    font-weight: 600;
}

.total-text {
    color: var(--text-muted);
}

.divider {
    color: #e2e8f0;
}

.progress-bg {
    width: 100%;
    height: 4px;
    background: #f1f5f9;
    border-radius: 2px;
    overflow: hidden;
}

.progress-fill {
    height: 100%;
    border-radius: 2px;
    transition: width 0.3s;
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

.btn-icon.delete:hover {
    color: var(--danger);
    background: #fee2e2;
}

/* 状态展示 */
.empty-state {
    text-align: center;
    padding: 3rem;
    color: var(--text-muted);
}

.loading-state {
    text-align: center;
    padding: 2rem;
    color: var(--text-muted);
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 10px;
}

.spinner {
    width: 20px;
    height: 20px;
    border: 2px solid #e2e8f0;
    border-top-color: var(--primary);
    border-radius: 50%;
    animation: spin 1s linear infinite;
}

@keyframes spin {
    to {
        transform: rotate(360deg);
    }
}

/* 响应式 */
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
}
</style>