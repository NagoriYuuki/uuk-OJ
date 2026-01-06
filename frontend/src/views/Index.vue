<template>
    <div style="padding: 16px">
        <h1>首页</h1>

        <div v-if="username">
            <p>你好，{{ username }}！</p>
            <p>
                <RouterLink :to="{ name: 'problem-list' }">进入题目列表</RouterLink>
                <br>
                <span v-if="isAdmin">
                    <RouterLink :to="{ name: 'problem-ma' }">题目管理</RouterLink>
                    <br>
                    <RouterLink :to="{ name: 'user-ma' }">用户管理</RouterLink>

                </span>
                <br>
                <a href="#" @click.prevent="logout">退出登录</a>

            </p>
        </div>

        <div v-else>
            <p>
                <RouterLink :to="{ name: 'problem-list' }">进入题目列表</RouterLink>
                <br>
                <RouterLink :to="{ name: 'login' }">登录！</RouterLink>
            </p>
        </div>


    </div>
</template>

<script setup lang="ts">
import { RouterLink, useRouter } from 'vue-router'
import { ref, onMounted } from 'vue'

const username = ref('')
const router = useRouter()
let isAdmin = ref(false)

onMounted(() => {
    username.value = localStorage.getItem('username') || ''
    checkAuth()
})

function logout() {
    localStorage.removeItem('token')
    localStorage.removeItem('username')
    username.value = ''
    alert('已退出登录')
    router.push('/index')
}

async function checkAuth() {
    const token = localStorage.getItem('token')
    if (!token)
        return
    try {
        const res = await fetch('/api/auth/userinfo', {
            headers: {
                'Authorization': `Bearer ${token}`
            }
        })

        if (res.ok) {
            const data = await res.json()
            isAdmin.value = data.role === 1
        }
    } catch (e) {
        console.error("Auth check failed", e)
    }
}

</script>