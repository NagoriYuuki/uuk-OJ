import { createRouter, createWebHistory } from 'vue-router'

import Index from '../views/Index.vue'
import ProblemList from '../views/ProblemList.vue'
import ProblemDetail from '../views/ProblemDetail.vue'
import Login from '../views/login.vue'
import Register from '../views/register.vue'
import ProblemManage from '../views/admin/problems.vue'
import UserManage from '../views/admin/users.vue'
import ProblemForm from '../views/admin/problemForm.vue'



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
        { path: '/problem-ma', name: 'problem-ma', component: ProblemManage },
        { path: '/user-ma', name: 'user-ma', component: UserManage },
        { path: '/admin/problems/create', name: 'problem-create', component: ProblemForm },
        { path: '/admin/problems/edit/:id', name: 'problem-edit', component: ProblemForm, props: true },
    ],
})

export default router
