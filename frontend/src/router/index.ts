import { createRouter, createWebHistory } from 'vue-router'

import Index from '../views/Index.vue'
import ProblemList from '../views/ProblemList.vue'
import ProblemDetail from '../views/ProblemDetail.vue'
import Login from '../views/login.vue'
import Register from '../views/register.vue'


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
    ],
})

export default router
