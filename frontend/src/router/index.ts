import { createRouter, createWebHistory } from 'vue-router'

import Index from '../views/Index.vue'
import ProblemList from '../views/ProblemList.vue'
import ProblemDetail from '../views/ProblemDetail.vue'
import Login from '../views/login.vue'
import Register from '../views/register.vue'
import ProblemManage from '../views/admin/problems.vue'
import UserManage from '../views/admin/users.vue'
import ProblemForm from '../views/admin/problemForm.vue'
import Submissions from '../views/Submissions.vue'
import SubmissionDetail from '../views/SubmissionDetail.vue'

import { authState, refreshUserInfo } from '../utils/auth'



const router = createRouter({
    history: createWebHistory(),
    routes: [
        { path: '/', name: 'index', component: Index },
        { path: '/index', redirect: '/' },
        { path: '/problems', name: 'problem-list', component: ProblemList },
        {
            path: '/problems/:id(\\d+)',
            name: 'problem-detail',
            component: ProblemDetail,
            props: (route) => ({ id: Number(route.params.id) }),
        },
        { path: '/login', name: 'login', component: Login },
        { path: '/register', name: 'register', component: Register },
        { path: '/submissions', name: 'submissions', component: Submissions },
        {
            path: '/submissions/:id(\\d+)',
            name: 'submission-detail',
            component: SubmissionDetail,
            props: (route) => ({ id: Number(route.params.id) }),
            meta: { requiresAuth: true },
        },
        { path: '/problem-ma', name: 'problem-ma', component: ProblemManage, meta: { requiresAdmin: true } },
        { path: '/user-ma', name: 'user-ma', component: UserManage, meta: { requiresAdmin: true } },
        { path: '/admin/problems/create', name: 'problem-create', component: ProblemForm, meta: { requiresAdmin: true } },
        { path: '/admin/problems/edit/:id', name: 'problem-edit', component: ProblemForm, props: true, meta: { requiresAdmin: true } },
    ],
})

router.beforeEach(async (to) => {
    if (authState.token && !authState.loaded) {
        await refreshUserInfo(false)
    }

    if (to.meta?.requiresAuth) {
        if (!authState.token) return { name: 'login', query: { redirect: to.fullPath } }
    }

    if (to.meta?.requiresAdmin) {
        if (!authState.token) return { name: 'login', query: { redirect: to.fullPath } }
        await refreshUserInfo(false)
        if (authState.role !== 1) return { name: 'index' }
    }
})

export default router
