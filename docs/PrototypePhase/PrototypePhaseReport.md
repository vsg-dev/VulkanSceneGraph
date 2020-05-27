# VulkanSceneGraph Prototype Phase Report

The objective of the three month Prototype Phase, October-December 2018, was to flesh out:

1. High level project systems to be used in software development and community support
2. Portability of code base and systems to ensure easy cross platform development
3. Range of functionality to be encompassed in core library
4. How best to support add on libraries and applications built on top of VSG.
5. Test different lower level design and implementation approaches
6. Build upon and refine the work carried out in the Exploration Phase to provide a more rounded base for the Core Development Phase that will begin in January 2019.

## 1. High level project systems
**Hosting:** CMake, C++17, Vulkan were chosen in the Exploration Phase as the core software technologies that the VulkanSceneGraph project would be based upon. Github was chosen as the venue for software development, during the Exploration Phase this was hosted as part private repository, then made public at the start of the Prototype Phase, and then finally a dedicated [https://github.com/vsg-dev](https://github.com/vsg-dev) github account was created for VulkanSceneGraph project work going forward.

**Website:** The focus of the first year of development on the VulkanSceneGraph will be software development which will limit how much time can be dedicated to creation of websites and supporting materials, the project still requires a conventional website interface so to minimize the time required to support a website [Github's Pages](https://help.github.com/articles/what-is-github-pages/) functionality was adopted that automatically builds a html website from the projects github repository.  This is limited in functionality compared to a full-blown website but for the purposes of the first year of work on VulkanSceneGraph it should be sufficient. The [vulkanscenegraph.org](http://www.vulkanscenegraph.org) domain was purchased and has been setup to redirect to the .io website: https://vsg-dev.github.io/VulkanSceneGraph/

**Social Media** : To help communicate with the wider software community a [https://twitter.com/dev_vsg](https://twitter.com/dev_vsg) twitter account was created and has been used for announcements for feature development.

**Community Discussion**: In preparation for a community of user/developers building upon around the VulkanSceneGraph project a [vsg-dev](https://groups.google.com/forum/#!forum/vsg-users) Google Group has been created.  Google Groups was chosen based on the ability to support both mailing list and forum interaction whilst minimizing the overhead in setting up and maintaining the list.  Later in the projects life it may be necessary to self host a community mailing list/forum.

