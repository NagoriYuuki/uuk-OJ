<template>
    <div style="padding: 16px">
        <h1>首页</h1>

        <div v-if="isLoggedIn">
            <p>你好，{{ displayName }}！</p>
            <p>
                <RouterLink :to="{ name: 'problem-list' }">进入题目列表</RouterLink>
                <br>
                <RouterLink :to="{ name: 'submissions' }">查看提交记录</RouterLink>
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
                <RouterLink :to="{ name: 'submissions' }">查看提交记录</RouterLink>
                <br>
                <RouterLink :to="{ name: 'login' }">登录！</RouterLink>
            </p>
        </div>


    </div>
</template>

<script setup lang="ts">
import { RouterLink, useRouter } from 'vue-router'
import { computed, onMounted } from 'vue'
import { authState, clearSession, refreshUserInfo } from '../utils/auth'

const router = useRouter()

const isLoggedIn = computed(() => !!authState.token)
const isAdmin = computed(() => authState.role === 1)
const displayName = computed(() => authState.username || (authState.userId ? `UID:${authState.userId}` : ''))

onMounted(() => {
    refreshUserInfo(false)
})

function logout() {
    clearSession()
    router.push('/')
}

</script>