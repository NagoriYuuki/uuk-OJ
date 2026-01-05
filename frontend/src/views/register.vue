<template>
    <div style="padding: 16px">
        <h1>注册</h1>
        <form @submit.prevent="handleRegister">
            <div>
                <label>用户名：</label>
                <input v-model="username" type="text" required />
            </div>
            <div>
                <label>密码：</label>
                <input v-model="password" type="password" required />
            </div>
            <div>
                <label>重复密码：</label>
                <input v-model="repassword" type="password" required />
            </div>
            <button type="submit" :disabled="loading">注册</button>
        </form>
        <p v-if="error" style="color: red">{{ error }}</p>
        <p>已有账号？<RouterLink to="/login">去登录！</RouterLink>
        </p>
        <br>
        <p>
            <RouterLink to="/Index">返回首页</RouterLink>
        </p>
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


        alert('注册成功！')
        router.push('/login')
    } catch (e: any) {
        error.value = e.message
    } finally {
        loading.value = false
    }
}
</script>