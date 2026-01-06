<template>
    <div style="padding: 16px">
        <h1>用户管理</h1>
        <p>
            <RouterLink to="/">返回首页</RouterLink>
        </p>

        <button @click="openCreate">新建用户</button>

        <table border="1" style="width: 100%; margin-top: 10px; border-collapse: collapse;">
            <thead>
                <tr>
                    <th>ID</th>
                    <th>用户名</th>
                    <th>角色</th>
                    <th>操作</th>
                </tr>
            </thead>
            <tbody>
                <tr v-for="u in users" :key="u.id">
                    <td>{{ u.id }}</td>
                    <td>{{ u.username }}</td>
                    <td>
                        <span v-if="u.role === 1" style="color: red; font-weight: bold;">管理员</span>
                        <span v-else>普通用户</span>
                    </td>
                    <td>
                        <button @click="openEdit(u)">编辑</button>
                        <button @click="deleteUser(u.id)" :disabled="u.id === currentUserId">删除</button>
                    </td>
                </tr>
            </tbody>
        </table>

        <!-- 用户编辑/新建弹窗 -->
        <div v-if="showModal" class="modal-overlay">
            <div class="modal">
                <h2>{{ isNew ? '新建用户' : '编辑用户' }}</h2>
                <form @submit.prevent="submitForm">
                    <div>
                        <label>用户名:</label>
                        <input v-model="form.username" required />
                    </div>
                    <div>
                        <label>密码:</label>
                        <input v-model="form.password" type="password" :placeholder="isNew ? '必填' : '留空则不修改'" :required="isNew" />
                    </div>
                    <div>
                        <label>角色:</label>
                        <select v-model.number="form.role">
                            <option :value="0">普通用户</option>
                            <option :value="1">管理员</option>
                        </select>
                    </div>
                    
                    <div style="margin-top: 20px; text-align: right;">
                        <button type="button" @click="closeModal" style="margin-right: 10px;">取消</button>
                        <button type="submit">保存</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
</template>

<style scoped>
.modal-overlay {
    position: fixed;
    top: 0; left: 0; width: 100%; height: 100%;
    background: rgba(0,0,0,0.5);
    display: flex;
    justify-content: center;
    align-items: center;
}
.modal {
    background: white;
    padding: 20px;
    width: 400px;
    border-radius: 8px;
    box-shadow: 0 2px 10px rgba(0,0,0,0.1);
}
.modal div {
    margin-bottom: 10px;
}
.modal label {
    display: block;
    margin-bottom: 5px;
}
.modal input, .modal select {
    width: 100%;
    padding: 8px;
    box-sizing: border-box;
}
</style>

<script setup lang="ts">
import { ref, onMounted, reactive } from 'vue'
import { RouterLink } from 'vue-router'

interface User {
    id: number
    username: string
    role: number
}

const users = ref<User[]>([])
const showModal = ref(false)
const isNew = ref(false)
const currentUserId = ref<number | null>(null)

const form = reactive({
    id: 0,
    username: '',
    password: '',
    role: 0
})

async function fetchUsers() {
    const token = localStorage.getItem('token')
    if (!token) return

    try {
        // 获取当前用户信息，以便知道哪个是自己（不能删除自己）
        const meRes = await fetch('/api/auth/userinfo', {
             headers: { 'Authorization': `Bearer ${token}` }
        })
        if (meRes.ok) {
            const meData = await meRes.json()
            currentUserId.value = meData.id
        }

        const res = await fetch('/api/admin/users', {
            headers: { 'Authorization': `Bearer ${token}` }
        })
        if (res.ok) {
            users.value = await res.json()
        } else {
            const data = await res.json().catch(() => ({}))
             alert('获取用户列表失败: ' + (data.message || res.statusText))
        }
    } catch (e) {
        console.error(e)
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
    form.password = '' // 编辑时不显示原密码
    form.role = u.role
    showModal.value = true
}

function closeModal() {
    showModal.value = false
}

async function submitForm() {
    const token = localStorage.getItem('token')
    if (!token) return alert('请先登录')
    
    const url = isNew.value ? '/api/admin/users' : `/api/admin/users/${form.id}`
    const method = isNew.value ? 'POST' : 'PUT'

    const payload: any = {
        username: form.username,
        role: form.role
    }
    // 仅当用户输入了密码时才发送密码字段
    if (form.password) {
        payload.password = form.password
    }

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
            alert('操作成功')
            closeModal()
            fetchUsers()
        } else {
            const data = await res.json().catch(() => ({}))
            alert('操作失败: ' + (data.message || res.statusText))
        }
    } catch (e) {
        console.error(e)
        alert('请求出错')
    }
}

async function deleteUser(id: number) {
    if (!confirm('确定要删除该用户吗? 此操作不可恢复！')) return
    const token = localStorage.getItem('token')
    
    try {
        const res = await fetch(`/api/admin/users/${id}`, {
            method: 'DELETE',
            headers: {
                'Authorization': `Bearer ${token}`
            }
        })
        if (res.ok) {
            alert('删除成功')
            fetchUsers()
        } else {
            const data = await res.json().catch(() => ({}))
            alert('删除失败: ' + (data.message || res.statusText))
        }
    } catch (e) {
        alert('请求出错')
    }
}

onMounted(() => {
    fetchUsers()
})
</script>