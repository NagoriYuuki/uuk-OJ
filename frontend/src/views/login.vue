<template>
    <div style="padding: 16px">
        <h1>登录</h1>
        <form @submit.prevent="handleLogin">
            <div>
                <label>用户名：</label>
                <input v-model="username" type="text" required />
            </div>
            <div>
                <label>密码：</label>
                <input v-model="password" type="password" required />
            </div>
            <button type="submit" :disabled="loading">登录</button>
        </form>
        <p v-if="error" style="color: red">{{ error }}</p>
        <p>还没有账号？<RouterLink to="/register">去注册</RouterLink>
        </p>
        <br>
        <p>
            <RouterLink to="/">返回首页</RouterLink>
        </p>
    </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { login } from '../utils/auth'

const username = ref('')
const password = ref('')
const loading = ref(false)
const error = ref('')
const router = useRouter()

async function handleLogin() {
    loading.value = true
    error.value = ''
    try {
        await login(username.value, password.value)
        const redirect = (router.currentRoute.value.query.redirect as string) || '/'
        router.push(redirect)
    } catch (e: any) {
        error.value = e.message
    } finally {
        loading.value = false
    }
}
</script>