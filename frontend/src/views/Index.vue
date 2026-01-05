<template>
    <div style="padding: 16px">
        <h1>首页</h1>
        <p>
            <RouterLink :to="{ name: 'problem-list' }">进入题目列表</RouterLink>
        </p>
    </div>
</template>

<script setup lang="ts">
import { RouterLink } from 'vue-router'
</script>